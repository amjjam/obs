/*============================================================================
  procssor [--receiver <string>] [--tracker <string>]
  
  --receiver-frames <string>
    Specifies a communicator for listening for data frames. Shared memory.
    <name>:<nbuffers>:<size>
    <name> is /<text> name of the shared memory and of a semphore
    <nbuffers> number of buffers
    <size> size of each buffer
  --sender-tracker <string>
    Specifies a communicator to the tracker
  --sender-phasorviewer <string>
    Specifies a communicator to the phasor viewer
  --framesize nL nF - number of pixels in wavelength and fringe direction
    Defaults are nL=256 and nF=320
  --fringeperiod p - number of pixels in one fringe period. Specify once
    for each baseline and specify in order to send to the tracker. Sign
    positive or negative
  --baseline name n L0 L1 A S d
    (<string> <int> <float> <float> <float> <float>)
    name - baseline name
    n - number of wavelength channels
    L0 - shortest wavelength in microns
    L1 - longest wavelength in microns
    A - amplitude of phasors
    S - standard deviation of noise in each of real and imaginary parts
    d - delay in microns
    (This is intended for testing)
  --fourier-baseline p
    p the period in pixels of the shortest wavelength channel
    (specify once for each baseline)
  --server-frames
    The address to which clients can connect to receive frames
    Default is TCP ":27012"
  --server-commands
    The address to which clients can connect to receive frames
    Default is TCP ":27013"
    Commands are:
    S - stop. Don't send phasors to the fringe tracker.
    P - phasors. Compute phasors from incoming frames. Subtract bias
        frame from incoming frames and compute phasors. 
    B - accumulate a bias frame. It is followed by a uint32 counting
        the number of frames to average. If another command arrives before
	B is completed, the number of actual frames accumulated will be used.
 ==========================================================================*/

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
int read_argv_baseline(char *argv[]);

std::string receiver_frames;
std::string sender_tracker;
std::string sender_phasorviewer;

int nL=256,nF=320;
std::vector<float> periods;

time_t t0=0;
time_t t1;

bool debug=false;
char state='S';
  

std::vector<amjCom::Session> server_commands_sessions;
std::string server_commands_address=":27013";
void server_commands_session(amjCom::Server, amjCom::Session);
void server_commands_status(amjCom::Server, amjCom::Status);
void server_commands_session_receive(amjCom::Session, amjCom::Packet &);
void server_commands_session_status(amjCom::Session, amjCom::Status);

std::vector<amjCom::Session> server_frames_sessions;
std::string server_frames_address=":27012";
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
amjTime rtime;
Frame<uint16_t> rframe;
double rfps=0; // Raw input frame rate

// Bias data
amjTime btime;
amjTime tmpbtime;
Frame<uint16_t> bframe;
Frame<double> tmpbframe;
int nBias,iBias;
bool hasBias=false;

// Difference frame - used to compute phasors
Frame<uint16_t> dframe;

// Simulated data - used for testing
amjTime stime;
Frame<uint16_t> sframe;
int simulator_seed=0;

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
  
  //std::thread reporter_thread(status_thread);
  
  amjPacket fpacket; // For input frames
  amjPacket ppacket; // For output phasors to tracker
  
  FourierCompute compute(nL,nF,periods);//{-10,-5});
  PhasorSets phasors;
  
  /* for frame counting */
  int rcounter=0;
  amjTime T0,T;
  T0.now();
  for(int i=0;;i++){
    // Wait for frame from shared memory
    T.now();
    r.receive(fpacket);

    // Count frames, fps
    rcounter++;
    if(T-T0>=1){
      rfps=rcounter;
      rcounter=0;
      T0=T;
    }

    if(debug)
      std::cout << "received" << std::endl;

    // Read frame from packet
    {
      std::lock_guard<std::mutex> lock(mutex);
      
      fpacket.reset();
      rtime.read(fpacket.read(rtime.size()));
      rframe.read1(fpacket.read(rframe.size1()));
      rframe.read2(fpacket.read(rframe.size2()));
      
      if(state=='S')
	continue;

      if(state=='B'){
	if(iBias==0){
	  tmpbtime=rtime;
	  tmpbframe=rframe;
	}
	else
	  tmpbframe+=rframe;
	iBias++;
	if(iBias==nBias){
	  bframe=tmpbframe/nBias;
	  btime=tmpbtime;
	  hasBias=true;
	  state='S';
	  iBias=0;
	}
	continue;
      }
    }
    
    // Subtract bias frame if we have one
    if(hasBias)
      dframe=rframe-bframe;
    else
      dframe=rframe;
    
    // Process frame
    compute.phasors(dframe,phasors);

    if(debug)
      std::cout << phasors[0][0].real() << " " << phasors[0][0].imag()
		<< std::endl;
    
    // Write phasors into packet
    ppacket.clear();
    rtime.write(ppacket.write(T.size()));
    ppacket << phasors;
    
    // Send results to tracker
    s.send(ppacket);
    
    // Send results to phasor viewer
    t1=time(NULL);    
    if(t1>t0){
      t0=t1;
      if(sender_phasorviewer.size()>0){
	p.send(ppacket);
      }
    }
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
    else if(strcmp(argv[i],"--framesize")==0){
      i++;
      nL=atoi(argv[i]);
      i++;
      nF=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--fringeperiod")==0){
      i++;
      periods.push_back(atof(argv[i]));
    }
    else if(strcmp(argv[i],"--baseline")==0){
      i+=read_argv_baseline(argv+i+1);
    }
    else if(strcmp(argv[i],"--server-frames")==0){
      i++;
      server_frames_address=argv[i];
    }
    else if(strcmp(argv[i],"--server-commands")==0){
      i++;
      server_commands_address=argv[i];
    }
    else{
      std::cerr << "unknown parameter: " << argv[i] << std::endl;
      abort();
    }
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
    if(c=='S'){
      state='S';
    }
    else if(c=='P'){
      state='P';
    }
    else if(c=='B'){
      state='B';
      memcpy(&nBias,p.read(sizeof(uint32_t)),sizeof(uint32_t));
      iBias=0;
    }
    else if(c=='U'){
      struct DataProcessorStatus status;
      status.state=state;
      status.hasBias=hasBias;
      status.nBias=nBias;
      status.biastime=btime;
      status.fps=rfps;
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
    rtime.write(q.write(rtime.size()));
    dframe.write(q.write(rframe.size()));
    S->send(q);
  }
  else if(p.data()[0]=='R'){
    //std::cout << "rframe: " << rframe.nL() << ", " << rframe.nF() << std::endl;
    rtime.write(q.write(rtime.size()));
    rframe.write(q.write(rframe.size()));
    S->send(q);
  }
  else if(p.data()[0]=='T'){
    simulate_frame();
    stime.write(q.write(stime.size()));
    sframe.write(q.write(sframe.size()));
    S->send(q);
  }
  else if(p.data()[0]=='B'){
    btime.write(q.write(btime.size()));
    bframe.write(q.write(bframe.size()));
    S->send(q);
  }
  else
    std::cout << "server_frames: receive: error: command="
	      << p.data()[0] << " is not recognized. Ignoring" << std::endl;
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
  stime.now();
  sframe.resize(SIMULATED_L,SIMULATED_F);
  
  for(int iL=0;iL<SIMULATED_L;iL++)
    for(int iF=0;iF<SIMULATED_F;iF++)
      sframe[iL][iF]=2000+1000*
	sin(2*M_PI*((double)iF+
		    (double)iL/2*sin(2*M_PI*(double)simulator_seed/100)
		    -(double)SIMULATED_F/2)/(double)SIMULATED_PERIOD);
  simulator_seed++;
}


// void status_thread(){
//   while(status_running.load()){
//     {
//       std::lock_guard<std::mutex> lock(mutex);
//       amjCom::Packet p;
//       processor_status.write(p.write(processor_status.size()));
//       std::cout << "status_thread: sending" << std::endl;
//       for(unsigned int i=0;i<server_commands_sessions.size();i++)
// 	server_commands_sessions[i]->send(p);
//     }
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//   }
// }
