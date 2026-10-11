#include <amjFourierFITS.H>
#include <amjCom/amjComSHM.H>
#include <amjCom/amjComTCP.H>
#include <amjArg.H>

// For logging
#include <amjLog.H>
#include <amjLogCom.H>

amjLogCom::Server logServer;

amjArg::Help help
({"Usage: ",
   "FourierStreamer",
   " [-T milliseconds] (file.fits)",
   "Options:",
   "  -T <ms>   Override FITS cadence in milliseconds",
   "  -p <int>  TCP port number for commands. Default 27015",
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

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <thread>

using amjFourier::Frame;
using amjFourier::FITS::Reader;
float cadenceOverride1=-1.0f;
float cadenceOverride2=-1.0f;
std::vector<std::string> filenames;
std::string sender_frames="/frames:2:400000";
int port=27015;

std::mutex mutex_file;
std::condition_variable files_ready;
std::atomic<bool> files_changed{false};

class FrameRate{
public:
  void start(){ worker=std::thread(&FrameRate::run,this); }
  ~FrameRate(){
    {
      std::lock_guard<std::mutex> lock(mutex);
      stopping=true;
    }
    ready.notify_one();
    if(worker.joinable()) worker.join();
  }
  void count(){
    std::lock_guard<std::mutex> lock(mutex);
    frames++;
  }
  uint32_t get(){
    std::lock_guard<std::mutex> lock(mutex);
    return framerate;
  }
private:
  // Called by the timer thread with mutex held.
  void update(){
    framerate=frames;
    frames=0;
  }
  void run(){
    std::unique_lock<std::mutex> lock(mutex);
    auto next=std::chrono::steady_clock::now()+std::chrono::seconds(1);
    while(!ready.wait_until(lock,next,[this]{ return stopping; })){
      update();
      next+=std::chrono::seconds(1);
      // Avoid rapid catch-up updates if the process was suspended.
      if(next<=std::chrono::steady_clock::now())
        next=std::chrono::steady_clock::now()+std::chrono::seconds(1);
    }
  }
  std::mutex mutex;
  std::condition_variable ready;
  uint32_t frames=0,framerate=0;
  bool stopping=false;
  std::thread worker;
};

FrameRate frame_rate;

void set_message(const std::string &);

void parse_args(int argc, char *argv[]);
void load(const std::vector<std::string>& files,
	  std::vector<Frame<int32_t>>& frames,float& cadenceMs,float requestedCadence);
double readCadenceMilliseconds(const Reader& reader);
std::vector<amjCom::Session> server_sessions;
void server_session(amjCom::Server,amjCom::Session S);
void server_status(amjCom::Server, amjCom::Status s);
void server_session_receive(amjCom::Session s, amjCom::Packet &p);
void server_session_status(amjCom::Session S, amjCom::Status s);

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
    amjLog::Config config;
    config.source="DataStreamer";
    config.filename="logs/streamer.log";
    amjLog::configure(config);
    amjLog::info("Starting DataStreamer");
    
    parse_args(argc,argv);
    amjLog::info("Command line supplied "+std::to_string(filenames.size())+" files");
    if(cadenceOverride2>0)
      amjLog::info("Command-line cadence override: "+std::to_string(cadenceOverride2)+" ms");
    
    std::string server_address=":"+std::to_string(port);
    amjLog::info("Server address="+server_address);
    
    amjCom::Server server=
      amjCom::TCP::create_server(server_address,server_session,server_status);
    frame_rate.start();
    server->start();

    while(true){
      std::vector<std::string> files;
      float requestedCadence;
      {
	std::unique_lock<std::mutex> lock(mutex_file);
        if(filenames.empty()) amjLog::info("Waiting for a file list");
	files_ready.wait(lock,[]{ return !filenames.empty(); });
	files=filenames;
        requestedCadence=cadenceOverride1;
	files_changed=false;
      }
      
      std::vector<Frame<int32_t>> frames;
      float cadenceMs;
      
      load(files,frames,cadenceMs,requestedCadence);
      if(files_changed){
        amjLog::info("A newer file list arrived during loading; loading the new selection");
        continue;
      }

      amjLog::info("Opening frame output: "+sender_frames);
      amjPacket packetf;
      amjComEndpointSHM senderf("",sender_frames);
      amjFourier::Frame<int16_t> framei;
      size_t iframe=0;
      Timer timer(cadenceMs);
      amjLog::info("Starting playback: "+std::to_string(frames.size())+
                   " frames, interval "+std::to_string(cadenceMs)+
                   " ms ("+std::to_string(1000.0/cadenceMs)+" frames/s); repeating");
      while(!files_changed){
	framei=frames[iframe];
	framei.time(amjTime::Now());
	packetf.clear();
	framei.write(packetf.write(framei.size()));
	amjLog::debug("Sending frame "+std::to_string(iframe));
	senderf.send(packetf);
	frame_rate.count();
	uint64_t n=timer.wait_next();
	if(n>1) amjLog::warn("Timer overrun: "+std::to_string(n)+" periods");
	iframe=(iframe+1)%frames.size();
      }
      amjLog::info("Playback interrupted by an updated file list");
    }
  }
  catch(const std::exception& e){
    // Keep stderr available even if opening/writing the log file failed.
    try{amjLog::error(e.what());}
    catch(...){std::cerr<<"Error: "<<e.what()<<'\n';}
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}

void set_message(const std::string& m){
  amjLog::info(m);
}

void parse_args(int argc,char *argv[]){
  help.help(argc,argv);
  
  for(int i=1;i<argc;i++)
    if(strcmp(argv[i],"-T")==0){
      if(++i>=argc) throw std::runtime_error("-T requires a value");
      cadenceOverride2=std::stod(argv[i]);
      if(cadenceOverride2<=0.0)
        throw std::runtime_error("-T must be greater than zero");
    }
    else if(strcmp(argv[i],"-p")==0){
      if(++i>=argc) throw std::runtime_error("-p requires a value");
      port=std::stoi(argv[i]);
    }
    else if(argv[i][0]=='-')
      throw std::runtime_error(std::string("unknown option: ")+argv[i]);
    else
      filenames.push_back(argv[i]);
}


void load(const std::vector<std::string>& files,
	  std::vector<Frame<int32_t>>& frames,float& cadenceMs,float requestedCadence){
  frames.clear();
  const auto loadStart=std::chrono::steady_clock::now();
  amjLog::info("Loading selection: "+std::to_string(files.size())+" files");

  size_t index=1;
  for(const auto& filename:files){
    const auto fileStart=std::chrono::steady_clock::now();
    Reader reader;
    set_message("Loading "+std::to_string(index)+"/"
		+std::to_string(files.size())+": "+filename);
    if(!reader.open(filename)){
      amjLog::error("Could not open "+filename);
      throw std::runtime_error("Could not open FITS file: "+filename);
    }
    
    amjLog::info("Reading "+filename+": "+std::to_string(reader.width())+
                 " x "+std::to_string(reader.height())+" pixels, "+
                 std::to_string(reader.depth())+" slices");

    std::vector<Frame<int32_t>> f;
    if(!reader.readSlices(f)){
      amjLog::error("Could not read "+filename);
      throw std::runtime_error("Could not read FITS file: "+filename);
    }
    
    if(f.empty()){
      amjLog::error("File is empty: "+filename);
      throw std::runtime_error("No frames in FITS file: "+filename);
    }
    
    if(frames.empty()){ // first file
      cadenceMs=readCadenceMilliseconds(reader);
      if(requestedCadence>0) cadenceMs=requestedCadence;
      if(cadenceOverride2>0) cadenceMs=cadenceOverride2;
      const std::string source=cadenceOverride2>0?"command line":
        requestedCadence>0?"GUI request":"first FITS file";
      amjLog::info("Playback interval: "+std::to_string(cadenceMs)+" ms, from "+source);
    }
    
    const auto fileMs=std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now()-fileStart).count();
    amjLog::info("Loaded "+std::to_string(index)+"/"+std::to_string(files.size())+
                 ": "+filename+"; "+std::to_string(f.size())+
                 " frames in "+std::to_string(fileMs)+" ms");
    
    frames.insert(frames.end(),
		  std::make_move_iterator(f.begin()),
		  std::make_move_iterator(f.end()));
    index++;
  }

  const auto totalMs=std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::steady_clock::now()-loadStart).count();
  amjLog::info("Loading complete: "+std::to_string(files.size())+" files, "+
               std::to_string(frames.size())+" total frames in "+
               std::to_string(totalMs)+" ms");
}

double readCadenceMilliseconds(const Reader& reader){
  double dit,mindit;
  if(!reader.readKey("HIERARCH ESO DET SEQ1 DIT",dit))
    throw std::runtime_error("FITS file has no DIT keyword");
  if(!reader.readKey("HIERARCH ESO DET SEQ1 MINDIT",mindit))
    throw std::runtime_error("FITS file has no MINDIT keyword");
  return (dit+mindit)*1000;
}

void server_session(amjCom::Server,amjCom::Session S){
  amjLog::info("New control connection");
  S->start([&](amjCom::Session S, amjCom::Packet &p){
    server_session_receive(S,p);},
    [&](amjCom::Session S, amjCom::Status s){
      server_session_status(S,s);});
  server_sessions.push_back(S);
}

void server_status(amjCom::Server, amjCom::Status s){
  if(s.error()) amjLog::error("Control server: "+s.errormessage());
  if(s.state()) amjLog::info("Control server: "+s.statedescription());
}

void server_session_receive(amjCom::Session s, amjCom::Packet &p){
  // Route logging before consuming the command byte. Other packets are untouched.
  try{
    if(logServer.receive(s,p)) return;
  }
  catch(const std::exception &e){
    // Keep transport errors local; do not generate a new remote log per retry.
    std::cerr<<"Log request: "<<e.what()<<'\n';
    return;
  }

  if(p.pos()<0 || size_t(p.pos())>=p.size()) return;
  char c=p.read(1)[0];
  if(c=='U'){
    uint32_t framerate=frame_rate.get();
    p.clear();
    write(p,c);
    write(p,framerate);
    if(s) s->send(p);
  }
  else if(c=='R'){
    {
      std::lock_guard<std::mutex> lock(mutex_file);
      read(p,cadenceOverride1);
      read(p,filenames);
      amjLog::info("Received file selection: "+std::to_string(filenames.size())+" files");
      if(filenames.empty()) amjLog::info("Empty selection requested; playback will return to waiting");
      if(cadenceOverride1>0)
        amjLog::info("Requested cadence override: "+std::to_string(cadenceOverride1)+" ms");
      else amjLog::info("Requested cadence: use FITS file timing");
      if(cadenceOverride2>0)
        amjLog::info("Command-line cadence override takes precedence: "+
                     std::to_string(cadenceOverride2)+" ms");
      files_changed=true;
    }
    files_ready.notify_one();
  }
}

void server_session_status(amjCom::Session, amjCom::Status s){
  if(s.error()) amjLog::error("Control connection: "+s.errormessage());
  if(s.state()) amjLog::info("Control connection: "+s.statedescription());
}
