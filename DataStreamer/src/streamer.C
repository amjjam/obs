#include <amjFourierFITS.H>
#include <amjCom/amjComSHM.H>
#include <amjArg.H>

amjArg::Help help
({"Usage: ",
   "FourierStreamer",
   " [-T milliseconds] file.fits",
   "Options:",
   "  -T <ms>   Override FITS cadence in milliseconds",
   "  -h        Show this help"
});

#include <sys/timerfd.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using amjFourier::Frame;
using amjFourier::FITS::Reader;
std::string filename;
double cadenceOverride = -1.0;
std::string sender_frames="/frames:2:400000";

void parse_args(int argc, char *argv[]);
double readCadenceMilliseconds(const Reader& reader);

class Timer{
public:
  Timer(double ms){
    int64_t ns=static_cast<int64_t>(ms*1e6);
    if(ns<=0) throw std::runtime_error("invalid timer period");
    fd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC);
    if(fd<0) throw std::runtime_error(std::string("timerfd_create failed: ")
				      +strerror(errno));
    itimerspec ts{};
    ts.it_interval={ns/1000000000LL,ns%1000000000LL};
    ts.it_value=ts.it_interval;
    if(timerfd_settime(fd,0,&ts,nullptr)<0){
      int e=errno;
      close(fd);
      throw std::runtime_error(std::string("timerfd_settime failed: ")
			       +strerror(e));
    }    
  }

  ~Timer(){ close(fd); }
  uint64_t wait_next(){
    uint64_t n;
    ssize_t nr;
    do nr=read(fd,&n,sizeof(n)); while(nr<0&&errno==EINTR);
    if(nr<0)
      throw std::runtime_error(std::string("timerfd read failed: ")
			       +strerror(errno));
    if(nr!=sizeof(n))
      throw std::runtime_error("short read from timerfd");
    return n;
  }
private:
  int fd;
};

int main(int argc, char* argv[]){
  try{
    parse_args(argc,argv);
    
    Reader reader;
    if(!reader.open(filename))
      throw std::runtime_error("Could not open FITS file: "+filename);
    
    std::cout << "FITS dimensions: " << reader.width() << " x "
	      << reader.height() << " x " << reader.depth() << '\n';
    
    if(reader.depth()==0)
      throw std::runtime_error("FITS cube contains no frames");
    
    std::vector<Frame<int32_t>> frames;
    
    if(!reader.readSlices(frames))
      throw std::runtime_error("Could not read FITS image cube");
    
    if(frames.empty())
      throw std::runtime_error("No frames were loaded");
    
    std::cout << "Loaded " << frames.size() << " frames\n";
    
    double cadenceMs;
    
    cadenceMs=readCadenceMilliseconds(reader);
    if(cadenceMs<=0.0)
      throw std::runtime_error("Invalid cadence read from FITS header");
    
    if(cadenceOverride>0.0){
      cadenceMs=cadenceOverride;
      std::cout << "Using command-line cadence: " << cadenceMs << " ms\n";
    }
    else
      std::cout << "Using FITS cadence: " << cadenceMs << " ms\n";
    
    amjPacket packetf;
    amjComEndpointSHM senderf("",sender_frames);
    amjFourier::Frame<int16_t> framei;
    size_t iframe=0;
    Timer timer(cadenceMs);
    while(true){
      framei=frames[iframe];
      framei.time(amjTime::Now());
      packetf.clear();
      framei.write(packetf.write(framei.size()));
      std::cout << "sent frame: " << iframe << "\n";
      senderf.send(packetf);
      uint64_t n=timer.wait_next();
      if(n>1) std::cerr << "Timer overrun: " << n << " periods\n";
      iframe=(iframe+1)%frames.size();
    }
  }
  catch(const std::exception& e){
    std::cerr << "Error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}

void parse_args(int argc, char *argv[]){
  help.help(argc,argv);
  
  for(int i=1;i<argc;i++)
    if(strcmp(argv[i],"-T")==0){
      if(++i>=argc) throw std::runtime_error("-T requires a value");
       cadenceOverride=std::stod(argv[i]);
      if(cadenceOverride<=0.0)
	throw std::runtime_error("-T must be greater than zero");
    }
    else if(argv[i][0]=='-')
      throw std::runtime_error(std::string("unknown option:")+argv[i]);
    else{
      if(!filename.empty())
	throw std::runtime_error(std::string("specify only one filename: ")
				 +filename+" already selected; "+
				 +argv[i]+" is extra.");
      filename=argv[i];
    }
  if(filename.empty()) throw std::runtime_error("no FITS filename specified");
}

double readCadenceMilliseconds(const Reader& reader){
  double dit,mindit;
  if(!reader.readKey("HIERARCH ESO DET SEQ1 DIT",dit))
    throw std::runtime_error("FITS file has no DIT keyword");
  if(!reader.readKey("HIERARCH ESO DET SEQ1 MINDIT",mindit))
    throw std::runtime_error("FITS file has no MINDIT keyword");
  return (dit+mindit)*1000;
}
