#include <amjArg.H>

amjArg::Help

/*============================================================================*/
help({
    "procssor [--receiver <string>] [--tracker <string>]",
    "",
    "--receiver-frames <string>",
    "  Specifies a communicator for listening for data frames. Shared memory.",
    "  <name>:<nbuffers>:<size> default is /frames:2:400000",
    "   <name> is /<text> name of the shared memory and of a semphore",
    "   <nbuffers> number of buffers",
    "   <size> size of each buffer",
    "--sender-tracker <string>",
    "  Specifies a communicator to the tracker",
    "  Default is 127.0.0.1:27001",
    "--sender-phasorviewer <string>",
    "  Specifies a communicator to the phasor viewer",
    "  Default is 127.0.0.1:27002",
    "--server-commands - TCP server for sending commands",
    "  Default :27013",
    "--server-frames - TCP server for getting frames",
    "  Default :27012",
    "--fringeperiod p - number of pixels in one fringe period. Specify once",
    "  for each baseline and specify in order to send to the tracker. Sign",
    "  positive or negative. If not specified, the default in the CalcPhasors",
    "  object is used.",
    "--baselinename <string> - name to attach each frame period. Specify once",
    "  for each fringe period, whether default in CalcPhasor or specified on",
    "  the command line with --fringeperiod. If not specified, the default in",
    "  CalcPhasors is used.",
    "--baseline name n L0 L1 A S d",
    "  (<string> <int> <float> <float> <float> <float>)",
    "  name - baseline name",
    "  n - number of wavelength channels",
    "  L0 - shortest wavelength in microns",
    "  L1 - longest wavelength in microns",
    "  A - amplitude of phasors",
    "  S - standard deviation of noise in each of real and imaginary parts",
    "  d - delay in microns",
    "  (This is intended for testing)",
    "--server-frames",
    "  The address to which clients can connect to receive frames",
    "  Default is TCP :27012",
    "--server-commands",
    "  The address to which clients can connect to send commands",
    "  Default is TCP :27013",
    "  Commands are:",
    "  S - stop. Don't send phasors to the fringe tracker.",
    "  P - phasors. Compute phasors from incoming frames. Subtract bias",
    "      frame from incoming frames and compute phasors.",
    "  B - accumulate a bias frame. It is followed by a uint32 counting",
    "      the number of frames to average. If another command arrives before",
    "  B is completed, the number of actual frames accumulated will be used.",
    "  F - File. Start or stop writing to file. It is followed by O to start,",
    "      and C to stop (open/close). O is followed C or F for counts or",
    "      frames.",
    "      That is followed by uint32_t and then characters. The",
    "      uint32_t is the number of characters which are the file name to",
    "      write to."});
/*==========================================================================*/

#include <cstring>
#include <iostream>
#include <complex>
#include <unistd.h>
#include <cstdlib>
#include <time.h>

#include <amjCom/amjComUDP.H>
#include <amjCom/amjComSHM.H>
#include <amjCom/amjComTCP.H>

#include <amjFourier.H>
#include <amjTime.H>

void parse_args(int argc, char *argv[]);
void write_to_file(amjFourier::Frame<int16_t>);
int read_argv_baseline(char *argv[]);

std::string receiver_frames="/frames:2:400000";
std::string sender_tracker="127.0.0.1:27001";
std::string sender_phasorviewer="127.0.0.1:27002";
std::string server_commands_address=":27013";
std::string server_frames_address=":27012";

std::vector<float> periods;
std::vector<std::string> names;

time_t t0=0;
time_t t1;

bool debug=false;
char state='S';

std::vector<amjCom::Session> server_commands_sessions;
void server_commands_session(amjCom::Server, amjCom::Session);
void server_commands_status(amjCom::Server, amjCom::Status);
void server_commands_session_receive(amjCom::Session, amjCom::Packet &);
void server_commands_session_status(amjCom::Session, amjCom::Status);

std::vector<amjCom::Session> server_frames_sessions;
void server_frames_session(amjCom::Server, amjCom::Session);
void server_frames_status(amjCom::Server, amjCom::Status);
void server_frames_session_receive(amjCom::Session S, amjCom::Packet &p);
void server_frames_session_status(amjCom::Session S, amjCom::Status s);

void simulate_frame();

// Status reporting
#include <thread>
#include <mutex>
#include <atomic>
#include "../include/DataProcessorStatus.H"
std::mutex mutex;
//std::atomic<bool> status_running{true};
struct DataProcessorStatus processor_status;
//void status_thread();
// Status reporting

// Raw input data
amjFourier::Frame<int16_t> rframe;
double fps=0; // Raw input frame rate

// Bias data
amjTime btime;
amjTime tmpbtime;
amjFourier::Frame<double> bframe;
amjFourier::Frame<double> tmpbframe;
int nBias,iBias;
bool hasBias=false;

// Difference frame - used to compute phasors
amjFourier::Frame<float> dframe;

// Simulated data - used for testing
amjFourier::Frame<int16_t> sframe;
int simulator_seed=0;

// For file output
int32_t fileframes=0;
std::string filename;
FILE *fp=nullptr;
char filetype=0;

int main(int argc, char *argv[]){
  parse_args(argc,argv);
  
  amjComEndpointSHM r(receiver_frames,"");
  amjComEndpointUDP s("",sender_tracker);
  amjComEndpointUDP p("",sender_phasorviewer);
  
  amjCom::Server server_commands=
    amjCom::TCP::create_server(server_commands_address,
			       server_commands_session,
			       server_commands_status);
  server_commands->start();
  amjCom::Server server_frames=
    amjCom::TCP::create_server(server_frames_address,
			       server_frames_session,
			       server_frames_status);
  server_frames->start();
  
  amjPacket fpacket; // For input frames
  amjPacket ppacket; // For output phasors to tracker

  amjFourier::CalcPhasors calcphasors;
  calcphasors.set_channels(150,106);
  
  std::vector<amjInterferometry::Phasors<> > phasors;
    
  // For counting fps
  int counter=0;
  amjTime T0,T;
  T0.now();
  for(int i=0;;i++){
    r.receive(fpacket);
    T.now(); // For couting fps
    std::cout << "fpacket.size()=" << fpacket.size() << std::endl;
    
    if(fpacket.size()==0){
      std::cerr << "Warning: received empty packet. Skipping." << std::endl;
      continue;
    }
    
    // Count frames, fps
    counter++;
    if(T-T0>=1){
      fps=counter;
      counter=0;
      T0=T;
    }
    
    // Read frame from packet
    {
      std::lock_guard<std::mutex> lock(mutex);
      
      fpacket.reset();
      rframe.read1(fpacket.read(rframe.size1()));
      rframe.read2(fpacket.read(rframe.size2()));
    }

    std::cout << "rframe.size()=" << rframe.size() << std::endl;
    
    if(state=='S') // Stopped, skip frame
      continue;
    
    {
      std::lock_guard<std::mutex> lock(mutex);
      
      if(state=='B'){ // Collecting bias frames
	if(iBias==0){
	  hasBias=false;
	  bframe=rframe;
	}
	else
	  bframe+=rframe;
	iBias++;
	if(iBias==nBias){
	  bframe/=nBias;
	  hasBias=true;
	  state='S';
	  iBias=0;
	}
	continue;
      }
    }
    
    // Subtract bias frame if we have one
    {
      std::lock_guard<std::mutex> lock(mutex);
      
      if(hasBias)
	dframe=amjFourier::Frame<float>(rframe)-bframe;  
      else
	dframe=rframe;
    }
    
    // Process frame
    calcphasors(dframe,phasors);
    
    // Write phasors into packet and send to tracker
    ppacket.clear();
    uint32_t n=phasors.size();
    memcpy(ppacket.write(sizeof(uint32_t)),&n,sizeof(uint32_t));
    for(uint32_t i=0;i<n;i++)
      phasors[i].write(ppacket.write(phasors[i].memsize()));
    s.send(ppacket);
    
    // Send phasors to phasor viewer
    t1=time(NULL);    
    if(t1>t0){
      t0=t1;
      if(sender_phasorviewer.size()>0)
	p.send(ppacket);
    }
    
    // Write to file if open, either counts or frames
    write_to_file(rframe);
  }
  
  return 0;
}

void parse_args(int argc, char *argv[]){
  
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--receiver-frames")==0){
      i++;
      receiver_frames=argv[i];
    }
    else if(strcmp(argv[i],"--sender-tracker")==0){
      i++;
      sender_tracker=argv[i];
    }
    else if(strcmp(argv[i],"--sender-phasorviewer")==0){
      i++;
      sender_phasorviewer=argv[i];
    }
    else if(strcmp(argv[i],"--server-commands")==0){
      i++;
      server_commands_address=argv[i];
    }
    else if(strcmp(argv[i],"--server-frames")==0){
      i++;
      server_frames_address=argv[i];
    }
    else if(strcmp(argv[i],"--fringeperiod")==0){
      i++;
      periods.push_back(atof(argv[i]));
    }
    else if(strcmp(argv[i],"--baselinename")==0){
      i++;
      names.push_back(argv[i]);
    }
    else if(strcmp(argv[i],"--baseline")==0){
      i+=read_argv_baseline(argv+i+1);
    }
    else{
      std::cerr << "unknown parameter: " << argv[i] << std::endl;
      abort();
    }
  }
}

void write_to_file(amjFourier::Frame<int16_t>){
  std::lock_guard<std::mutex> lock(mutex);
  if(fp){
    uint8_t buffer[20];
    rframe.time().write(buffer);
    fwrite(buffer,1,rframe.time().size(),fp);
    uint32_t wL=rframe.wL(),wF=rframe.wF(),iL,iF;
    if(filetype=='C'){ // Write counts to file
      uint32_t counts=0;
      for(iL=0;iL<wL;iL++)
	for(iF=0;iF<wF;iF++)
	  counts+=rframe[iL][iF];
      fwrite(&counts,sizeof(uint32_t),1,fp);
    }
    else if(filetype=='F'){ // Write frames to file
      fwrite(&wL,sizeof(uint32_t),1,fp);
      fwrite(&wF,sizeof(uint32_t),1,fp);
      for(iL=0;iL<wL;iL++)
	fwrite(&rframe[iL][0],sizeof(uint16_t),wF,fp);
    }
    fileframes++;
  }
}

int read_argv_baseline(char *argv[]){
  // std::string name=argv[0];
  // int n=atoi(argv[1]);
  // float L0=atof(argv[2]);
  // float L1=atof(argv[3]);
  // float A=atof(argv[4]);
  // float S=atof(argv[5]);
  // float d=atof(argv[6]);
  // sim.push_back(DataProcessorBaselineSim(name,n,L0,L1,A,S,d));
  
  return 7;
}

void server_commands_session(amjCom::Server, amjCom::Session S){
  std::cout << "new server commands session" << std::endl;
  S->start([&](amjCom::Session S, amjCom::Packet &p){
    server_commands_session_receive(S,p);},
    [&](amjCom::Session S, amjCom::Status s){
      server_commands_session_status(S,s);});
  server_commands_sessions.push_back(S);
}

void server_commands_status(amjCom::Server, amjCom::Status s){
  if(s.error()){
    std::cout << "DataProcessor: server_commands: Error: "
	      << s.errormessage() << std::endl;
  }

  if(s.state())
    std::cout << "DataProcessor: server_commands: State: "
	      << s.statedescription() << std::endl;
}

void server_commands_session_receive(amjCom::Session s, amjCom::Packet &p){
  char c=p.read(1)[0];
  
  {
    std::lock_guard<std::mutex> lock(mutex);
    if(c=='S'){ // Stop
      state='S';
    }
    else if(c=='P'){ // Compute phasors and send to tracker
      state='P';
    }
    else if(c=='B'){ // Collect bias data, then stop
      state='B';
      memcpy(&nBias,p.read(sizeof(uint32_t)),sizeof(uint32_t));
      iBias=0;
    }
    else if(c=='F'){ // File operation
      c=p.read(1)[0];
      if(c=='O'){ // Open file
	fileframes=0;
	uint32_t l;
	filetype=p.read(1)[0];
	memcpy(&l,p.read(sizeof(uint32_t)),sizeof(uint32_t));
	std::string filename((char *)p.read(l),l);
	if((fp=fopen(filename.c_str(),"w"))==nullptr){
	  std::cout << "warning: could not open file for output: "
		    << filename << std::endl;
	  fileframes=-1;
	}
      }
      else if(c=='C'){ // Close file
	if(fp!=nullptr)
	  fclose(fp);
	fp=nullptr;
      }
      else{
	std::cout << "error: command F followed by " << c << std::endl;
	exit(1);
      }
    }
    else if(c=='U'){ // Update
      struct DataProcessorStatus status;
      status.state=state;
      status.hasBias=hasBias;
      status.nBias=nBias;
      status.biastime=btime;
      status.fps=fps;
      status.fileframes=fileframes;
      p.clear();
      status.write(p.write(status.size()));
      s->send(p);
    }
    else{
      std::cout << "DataProcessor: server_commands: unknown command: "
		<< c << ". Ignoring" << std::endl;
    }
  }
}

void server_commands_session_status(amjCom::Session, amjCom::Status s){
  if(s.error()){
    std::cout << "DataProcessor: server_commands: Error: "
	      << s.errormessage() << std::endl;
  }
  
  if(s.state())
    std::cout << "DataProcessor: server_commands: State: "
	      << s.statedescription() << std::endl;
}

void server_frames_session(amjCom::Server, amjCom::Session S){
  std::cout << "new frames session" << std::endl;
  S->drop_pending(true);
  //std::static_pointer_cast<amjCom::TCP::_Session>(S)->drop_pending(true);
  S->start([&](amjCom::Session S, amjCom::Packet &p){
    server_frames_session_receive(S,p);},
    [&](amjCom::Session S, amjCom::Status s){
      server_frames_session_status(S,s);});
  server_frames_sessions.push_back(S); 
}

void server_frames_status(amjCom::Server, amjCom::Status s){
  if(s.error()!=amjCom::Error::NoError){
    std::cout << "DataProcessor: server_frames: Error: "
	      << s.errormessage() << std::endl;
    exit(1);
  }
  
  if(s.state()!=amjCom::State::NoState)
    std::cout << "DataProcessor: server_frames: State: "
	      << s.statedescription() << std::endl;
}

void server_frames_session_receive(amjCom::Session S, amjCom::Packet &p){
  if(p.size()!=1){
    std::cout << "server_frames: receive: error: size=" << p.size()
	      << ", expected size=1. Ignoring." << std::endl;
    return;
  }
  //std::cout << "server_frames_session_receive: " << p.data()[0] << std::endl;

  amjCom::Packet q;
  if(p.data()[0]=='D'){
    amjFourier::Frame<float> tframe(dframe);
    tframe.write(q.write(tframe.size()));
    S->send(q);
  }
  else if(p.data()[0]=='R'){
    amjFourier::Frame<float> tframe(rframe);
    std::cout << "rframe.size()=" << rframe.size() << ", tframe.size()=" << tframe.size() << std::endl;
    tframe.write(q.write(tframe.size()));
    S->send(q);
  }
  else if(p.data()[0]=='T'){
    simulate_frame();
    amjFourier::Frame<float> tframe(sframe);
    std::cout << "T: " << tframe.wL() << ", " << tframe.wF() << std::endl;
    std::cout << tframe.write(q.write(tframe.size())) << std::endl;
    S->send(q);
  }
  else if(p.data()[0]=='B'){
    amjFourier::Frame<float> tframe(bframe);
    tframe.write(q.write(tframe.size()));
    S->send(q);
  }
  else{
    std::cout << "server_frames: receive: error: command="
	      << p.data()[0] << " is not recognized. Ignoring" << std::endl;
    return;
  }

  std::cout << "sent frame: size=" << q.size() << std::endl;
  for(unsigned int i=0;i<q.size()&&i<25;i++)
    printf("%02x ",q.data()[i]);
  printf("\n");
}


void server_frames_session_status(amjCom::Session S, amjCom::Status s){
  std::cout << "server_frames_session_status:" << std::endl;
  std::cout << "state=" << s.statedescription() << std::endl;
  std::cout << "error=" << s.errormessage() << std::endl;
}

#define SIMULATED_L 256
#define SIMULATED_F 320
#define SIMULATED_PERIOD 100
void simulate_frame(){
  sframe.resize(SIMULATED_L,SIMULATED_F);
  sframe.L0(0);
  sframe.F0(0);
  sframe.nL(SIMULATED_L);
  sframe.nF(SIMULATED_F);
  sframe.time().now();
  
  for(int iL=0;iL<SIMULATED_L;iL++)
    for(int iF=0;iF<SIMULATED_F;iF++)
      sframe[iL][iF]=2000+1000*
	sin(2*M_PI*((double)iF+
		    (double)iL/2*sin(2*M_PI*(double)simulator_seed/100)
		    -(double)SIMULATED_F/2)/(double)SIMULATED_PERIOD);
  simulator_seed++;
}
