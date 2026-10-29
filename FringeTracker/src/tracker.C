#include <amjArg.H>

amjArg::Help
/*============================================================================*/
help({
    "tracker [--<switch> <options>]",
    "",
    "--baseline <name> <nchans> <lambda0> <lambda1> <nOver> <nIncoherent> <nSmooth>",
    "  Defines a baseline with name <name> it has <nchans> wavelength channels",
    "  equally-spaced in wavenumber between shorter wavelength <lambda0> and",
    "  longer wavelength <lambda1> both in micrometers. <nOver> is the",
    "  oversampling of the FFT, <nincoherent> is the number of consecutive",
    "  power spectra averaged, and <nSmooth> is the number of consecutive",
    "  delays smoothed.",
    "--receiver-phasors <string>",
    "  Specifies a communicator for listening for phasors",
    "  Default is 127.0.0.1:27001",
    "--sender-movements <string>",
    "  Specifies a communicator for sending delay line movements",
    "--delaylines <int>",
    "  Is the number of delaylines. Default is 6.",
    "--delaylines-index {<int>}",
    "  It is assumed that the baselines are in linear order, but the delay",
    "  lines which correspond to endpoints can be out of order. This is the",
    "  index to the delay line and should be one more than the number of",
    "  baselines. Default is 0, 1, 2, ....",
    // "--sender-tracker-stats <string> <int> <int>",
    // "  Specifies a communicator for sending fringe tracker statistics (first),",
    // "  and a sampling interval in ms (second), and a reporting interval in ms",
    // "  (third). Defaults for intervals are 5 ms and 1000 ms",
    // "--sender-pspec <string> <int>",
    // "  Specifies a communicator and a time interval in ms. Sends the delay",
    // "  machine average power spectrum via the communicator at the ms interval.",
    // "--receiver-tracker-controller <string>",
    // "  Specifies a communicator for receiving commands",
    // "--sender-tracker-controller <string> <int>",
    // "  Specifies a communicator for sending tracker status (first), and an",
    // "  interval in ms (second)",
    // "--sender-tracker-snr <string>",
    // "  Specifies a communicator for sending SNR",
    "--active <int>",
    "  <int> the state machines should begin in the active (1) or inactive (0)",
    "  state. Default is inactive (0).",
    "  The state machines should begin in the inactive state",
    "--trackerlog <string> a file into which the state machine state is logged",
    "  every time it executes. A header is written before the first record",
    "  <int32> the number of baselines (nB)",
    "  <int32> the number of delaylines (nD)",
    "  <int32> the number of states (nS), numbered 0 to K-1",
    "  nB of",
    "    <string> null-terminated strings of each baseline name",
    "  nS of",
    "    <string> null-terminated strings of each state name",
    "  Each time the state machine runs",
    "  <int32> <int32> the time in s and ns in the last frame",
    "  <int32> <int32> the time of recording the state in s and ns",
    "  nB of",
    "    <uint8> - the state (0 stop, 1, searching, 2 found, 3 locked, 4 lost)",
    "    <float> - the SNR found level",
    "    <float> - the SNR lost level",
    "    <float> - the current SNR",
    "  nD of",
    "    <float> - the movement for each delay line",
    "--listen <int>",
    "  TCP port to listen on for everything. For each new connection a single",
    "  character is expected to indicate the type of connection",
    "  C is a connection from a TrackerController GUI",
    "  F is a connection from Fringe Tracker Viewer GUI",
    "  P is a connection from a Power Spectrum Viewer GUI",
    "  H is a connection from a PhasorViewer GUI",
    "  S is a connection from a SNR Viewer GUI"
    /* ======================================================================*/
  });

#include "../include/FringeTrackerStateMachine.H"
#include "../include/DelayMachineDFT.H"
#include "../include/Baseline2DelaylineLinear.H"
#include "../include/FringeTrackerBaselineSpec.H"

#include "../../shared/include/Timing.H"

#include <mutex>
#include <semaphore.h>
#include <string>
#include <utility>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <fstream>

#include <amjCom/amjComUDP.H>
#include <amjCom/amjComTCP.H>
#include <amjCom/amjComSerialize.H>
#include <amjTime.H>
#include <amjInterferometry.H>

void parse_args(int,char *[]);
bool compare(const std::vector<amjInterferometry::Phasors<float> > &,
	     const std::vector<DelayMachineDFT> &);
bool compare(const std::vector<DelayMachineDFT> &,
	     const std::vector<FringeTrackerStateMachine> &);
void* sample_pspec(void *);
void* sample_tracker_stats(void *);
void resizetrackerstats();
void *tracker_controller_receiver(void *);
void *tracker_controller_sender(void *);
void send2SNRViewer(std::vector<DelayMachineDFT> &);

std::string receiver_phasors="127.0.0.1:27001";
std::string sender_movements;
std::string sender_pspec;
unsigned int sender_pspec_interval;
std::string sender_tracker_stats;
unsigned int sender_tracker_stats_sample_interval;
unsigned int sender_tracker_stats_report_interval;
std::string receiver_tracker_controller;
std::string sender_tracker_controller;
unsigned int sender_tracker_controller_interval;
//std::string sender_tracker_snr="127.0.0.1:27011";

int nDelaylines=6;
int stateMachineTSample=10;
int stateMachineTReport=1000;
bool active=false;

int nIncoherent=5;
int nSmooth=1;

std::vector<DelayMachineDFT> delayMachines;
int32_t version_delayMachines=0;
std::mutex mutex_delayMachines;

std::vector<FringeTrackerStateMachine> stateMachines;
int32_t version_stateMachines=0;
std::mutex mutex_stateMachines;

//PhasorSets phasors;
std::vector<amjInterferometry::Phasors<float> > phasors;
std::vector<std::pair<float,float> > baselinemovements;

// For state machine sampling and reporting
std::vector<std::string> baselinenames;
std::vector<std::string> statenames;
std::vector<FringeTrackerStateMachineStatistics> stats;
bool flag_resizetrackerstats=true;

// For logging
std::string trackerlog;
std::ofstream fplog;
void sigterm_callback(int s){
  if(fplog.is_open())
    fplog.close();
  exit(0);
}

Baseline2DelaylineLinear baseline2delayline(3,nDelaylines);

// TCP server
std::string server_address=":27014";
std::vector<amjCom::Session> server_sessions;
void server_session(amjCom::Server,amjCom::Session);
void server_status(amjCom::Server,amjCom::Status);
void server_session_receive(amjCom::Session,amjCom::Packet &);
void server_session_status(amjCom::Session,amjCom::Status);

// Continuously running counter
uint64_t frame_counter=0;
uint32_t frame_rate=0;
std::mutex mutex_frame_counter;

int main(int argc, char *argv[]){
  parse_args(argc,argv);

  // This should remain, to sample tracker stats, but should not send anything
  // Create a thread to sample the fringe trackers states and send statistics
  pthread_t thread_tracker_stats;
  if(pthread_create(&thread_tracker_stats,nullptr,sample_tracker_stats,
		    nullptr)!=0){
    perror("could not start thread_tracker_stats");
    exit(EXIT_FAILURE);
  }
  
  //std::cout << "sender_tracker_snr=" << sender_tracker_snr << std::endl;
  amjComEndpointUDP r(receiver_phasors,"");
  amjComEndpointUDP s("",sender_movements);
  //amjComEndpointUDP ssnr("",sender_tracker_snr);
  
  amjPacket packet;
  amjPacket packetSNRViewer;
  amjTime T;
  
  amjCom::Server server=amjCom::TCP::create_server(server_address,
						   server_session,
						   server_status);
  server->start();
  
  // Open trackerlog and write header information
  if(trackerlog.size()>0){
    fplog.open(trackerlog.c_str(),std::ios::binary|std::ios::out);
    int32_t nBaselines=stateMachines.size();
    fplog.write((char *)&nBaselines,sizeof(int32_t));
    fplog.write((char *)&nDelaylines,sizeof(int));
    for(unsigned int i=0;i<stateMachines.size();i++)
      fplog.write((char *)stateMachines[i].name().c_str(),
		  stateMachines[i].name().size()+1);
    for(int i=0;i<stateMachines[0].nstates();i++)
      fplog.write((char *)stateMachines[0].stateName(i).c_str(),
		  stateMachines[0].stateName(i).size()+1);
  }

  // For cleanly closing the log file on SIGTERM
  signal(SIGTERM,&sigterm_callback);
  
  while(true){
    packet.clear();
    r.receive(packet);
    uint32_t nphasors;
    memcpy(&nphasors,packet.read(sizeof(uint32_t)),sizeof(uint32_t));
    phasors.resize(nphasors);
    for(size_t j=0;j<phasors.size();j++){
      phasors[j].read1(packet.read(phasors[j].memsize1()));
      phasors[j].read2(packet.read(phasors[j].memsize2()));
    }

    if(frame_counter%100==0)
      std::cout << "nframes/100=" << frame_counter/100 << std::endl;

    // If there is a mismatch between phasors and delayMachines,
    // recreate delayMachines
    
    if(!compare(phasors,delayMachines)){
      std::lock_guard<std::mutex> lock(mutex_delayMachines);
      delayMachines.clear();
      for(size_t i=0;i<phasors.size();i++)
	delayMachines.push_back(DelayMachineDFT(phasors[i].name(),
						nIncoherent,nSmooth));
      version_delayMachines++;
    }
    
    // Load phasors into delay machines
    {
      std::lock_guard<std::mutex> lock(mutex_delayMachines);
      for(unsigned int i=0;i<phasors.size();i++)
	delayMachines[i].load(phasors[i]);
    }
    
    if(!compare(delayMachines,stateMachines)){
      std::lock_guard<std::mutex> lock(mutex_stateMachines);
      int state;
      if(active)
	state=STATE_SEARCH;
      else
	state=STATE_STOP;
      stateMachines.clear();
      for(size_t i=0;i<delayMachines.size();i++)
	stateMachines.push_back(FringeTrackerStateMachine(delayMachines[i].name(),
							  state));
      version_stateMachines++;
    }
    
    // Load delays into state machines
    {
      std::lock_guard<std::mutex> lock(mutex_stateMachines);
      for(unsigned int i=0;i<delayMachines.size();i++){
	stateMachines[i].loadDelay(delayMachines[i].delay(),
				   delayMachines[i].snr());
	stateMachines[i].advance();
      }
    }
    
    // Load baseline movements into delay line movement calculator
    Delays<float> baselinemovements(stateMachines.size());
    for(unsigned int i=0;i<stateMachines.size();i++)
      baselinemovements[i]=-stateMachines[i].movement();
    baseline2delayline.loadBaselineMovements(baselinemovements);
      
    // Send baseline movements to delay line interface
    Delays<float> delaylinemovements=baseline2delayline.delaylineMovements();

    packet.clear();
    T.write(packet.write(T.size()));
    packet << delaylinemovements;
    s.send(packet);

    // If the log file is open write an entry to it
    if(fplog.is_open()){
      struct timespec t=T.toTimespec();
      int32_t tt=t.tv_sec;
      fplog.write((char *)&tt,sizeof(int32_t));
      tt=t.tv_nsec;
      fplog.write((char *)&tt,sizeof(int32_t));
      for(unsigned int i=0;i<stateMachines.size();i++){
	uint8_t state=stateMachines[i].state();
	fplog.write((char *)&state,sizeof(uint8_t));
	fplog.write((char *)&stateMachines[i].getConfig().snrFound,sizeof(float));
	fplog.write((char *)&stateMachines[i].getConfig().snrLost,sizeof(float));
	float snr=stateMachines[i].snr();
	fplog.write((char *)&snr,sizeof(float));
      }
      for(int i=0;i<nDelaylines;i++)
	fplog.write((char *)&baselinemovements[i],sizeof(float));
    }
    std::lock_guard<std::mutex> lock(mutex_frame_counter);
    frame_counter++;
  }
    
  return 0;
}


void parse_args(int argc, char *argv[]){
  help.help(argc,argv);
  
  //std::vector<FringeTrackerBaselineSpec> baselinespecs;
  
  for(int i=1;i<argc;i++){
    /*if(strcmp(argv[i],"--baseline")==0){
      baselinespecs.push_back(FringeTrackerBaselineSpec(argv[i+1],atoi(argv[i+2]),atof(argv[i+3]),atof(argv[i+4]),atoi(argv[i+5]),atoi(argv[i+6]),atoi(argv[i+7])));
      i+=7;
    }
    else*/
    if(strcmp(argv[i],"--receiver-phasors")==0){
      i++;
      receiver_phasors=argv[i];
    }
    else if(strcmp(argv[i],"--sender-movements")==0){
      i++;
      sender_movements=argv[i];
    }
    else if(strcmp(argv[i],"--delayline")==0){
      i++;
      nDelaylines=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--state-machine-stats")==0){
      i++;
      stateMachineTSample=atoi(argv[i]);
      stateMachineTReport=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--sender-tracker-stats")==0){
      i++;
      sender_tracker_stats=argv[i];
      i++;
      sender_tracker_stats_sample_interval=atoi(argv[i]);
      i++;
      sender_tracker_stats_report_interval=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--sender-pspec")==0){
      i++;
      sender_pspec=argv[i];
      i++;
      sender_pspec_interval=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--receiver-tracker-controller")==0){
      i++;
      receiver_tracker_controller=argv[i];
    }
    else if(strcmp(argv[i],"--sender-tracker-controller")==0){
      i++;
      sender_tracker_controller=argv[i];
      i++;
      sender_tracker_controller_interval=atoi(argv[i]);
    }
    // else if(strcmp(argv[i],"--sender-tracker-snr")==0){
    //   i++;
    //   sender_tracker_snr=argv[i];
    // }
    else if(strcmp(argv[i],"--active")==0){
      i++;
      if(atoi(argv[i])==0)
	active=false;
      else if(atoi(argv[i])==1)
	active=true;
      else{
	std::cout << "tracker: invalid --active parameter: " << argv[i]
		  << std::endl;
	exit(EXIT_FAILURE);
      }
    }
    else if(strcmp(argv[i],"--trackerlog")==0){
      i++;
      trackerlog=argv[i];
    }
    else{
      std::cout << "unrecognized parameter: " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // if(baselinespecs.size()>0)
  //   setup_fringe_tracker(baselinespecs);

}

bool compare(const std::vector<amjInterferometry::Phasors<float> > &phasors,
	     const std::vector<DelayMachineDFT> &delayMachines){
  if(phasors.size()!=delayMachines.size())
    return false;
  
  for(size_t i=0;i<phasors.size();i++)
    if(phasors[i].name()!=delayMachines[i].name())
      return false;
  
  return true;
}

bool compare(const std::vector<DelayMachineDFT> &delayMachines,
	     const std::vector<FringeTrackerStateMachine> &stateMachines){
  if(stateMachines.size()!=delayMachines.size())
    return false;

  for(size_t i=0;i<delayMachines.size();i++)
    if(delayMachines[i].name()!=stateMachines[i].name())
      return false;

  return true;
}


// This should remain, to sample tracker stats, but it should not send anything
void *sample_tracker_stats(void *dummy){
  uint64_t old_frame_counter;
  {
    std::lock_guard<std::mutex> lock(mutex_frame_counter);
    old_frame_counter=frame_counter;
  }
  for(;;){
    {
      std::lock_guard<std::mutex> lock(mutex_frame_counter);
      frame_rate=(uint32_t)(frame_counter-old_frame_counter);
      old_frame_counter=frame_counter;
    }
    sleep(1);
  }
  
}

void server_session(amjCom::Server,amjCom::Session S){
  std::cout << "server_session: new session" << std::endl;
  S->start([&](amjCom::Session S, amjCom::Packet &p){
    server_session_receive(S,p);},
    [&](amjCom::Session S, amjCom::Status s){
      server_session_status(S,s);});
  server_sessions.push_back(S);
}

void server_status(amjCom::Server,amjCom::Status s){
  if(s.error()){
    std::cout << "FringeTracker: server: Error: "
	      << s.errormessage() << std::endl;
  }
  
  if(s.state())
    std::cout << "FringeTracker: server: State: "
	      << s.statedescription() << std::endl;

}

void server_session_receive(amjCom::Session s, amjCom::Packet &p){
  char c=p.read(1)[0];
  amjCom::Packet q;

  if(c=='P'){ // Send power spectra
    std::lock_guard<std::mutex> lock(mutex_delayMachines);
    uint32_t n=delayMachines.size();
    memcpy(q.write(sizeof(uint32_t)),&n,sizeof(uint32_t));
    for(unsigned int i=0;i<delayMachines.size();i++)
      delayMachines[i].p().write(q.write(delayMachines[i].p().memsize()));
  }
  else if(c=='S'){ // Send SNR values
    uint32_t version;
    memcpy(&version,p.read(sizeof(uint32_t)),sizeof(uint32_t)); 
    std::lock_guard<std::mutex> lock(mutex_delayMachines);
    uint32_t n=delayMachines.size();
    if(version!=version_delayMachines){
      c='U';
      write(q,c);
      write(q,version_delayMachines);
      std::vector<std::string> names;
      names.reserve(n);
      for(const auto &dm: delayMachines)
	names.push_back(dm.name());
      write(q,names);
    }
    c='D';
    write(q,c);
    amjTime time;
    if(n>0){
      time=delayMachines[0].time();
    }
    else
      time.now();
    time.write(q.write(time.size()));
    std::vector<float> snrs;
    snrs.reserve(n);
    for(auto &dm:delayMachines)
      snrs.push_back(dm.snr());
    write(q,snrs);
  }
    /*********************************************************
Communication between FringeTracker and FringeTrackerGUI FringeTracker
never initiates communication FringeTrackerGUI will always send 'C' as
the first character of a packet, followed by a version number. If
version number received by FringeTracker does no match, it sends back
a packet starting with 'C' and list of baselinenames, and ignores the
rest of a received packet. If FringeTrackerGUI receives a packet with
'C' it updates the list of baselines, and then sends all configuration
information to FringeTracker. FringeTracker will always respond to a
packet from FringeTrackerGUI, including one containing just a '\0'
section, and the first section will always be a 'U' section.

Format of packet from FringeTrackerGUI:
'C'
version number
multiple sections each started with one character:
               '\0' - end of packet.
               'D' - update of delay machines
               'S' - update of state machines

Following '\0': nothing, packet end
Following 'D': uint32_t nOver
               uint32_t nCoh
               uint32_t nIncoh
               uint32_t nSmooth
               uint32_t Leaky;
Following 'S': vector FringeTrackerStateMachineConfig

Format of packet from FringTracker:
multiple sections each started with one character:
               '\0' - end of packet
               'C' - baseline configuration
               'U' - update

Following 'C': int32_t version number
               std::vector<std::string> baseline names

Following 'U': amjTime latest time from delay machines (or current time if there are no delay machines)
               uint32_t frame rate

    *********************************************************/
  else if(c=='C'){ // Configuration information
    uint32_t c_version;
    read(p,c_version);
    std::lock_guard<std::mutex> lock(mutex_delayMachines);
    // If controller has old version send configuration
    if(c_version!=version_delayMachines){ 
      c='C';
      write(q,c);
      write(q,version_delayMachines);
      std::vector<std::string> names;
      names.reserve(delayMachines.size());
      for(const auto &dm: delayMachines)
	names.push_back(dm.name());
      write(q,names);      
    }
    else{
      read(p,c);
      if(c=='D'){ // Configuration of delay machine
	

      }
      if(c=='S'){ // Configuration of state machine


      }
    }
    c='U';
    write(q,c);
    amjTime T;
    if(delayMachines.size()>0)
      T=delayMachines[0].time();
    else
      T.now();
    T.write(q.write(T.size()));
    uint32_t copy_frame_rate;
    {
      std::lock_guard<std::mutex> lock(mutex_frame_counter);
      copy_frame_rate=frame_rate;
    }
    write(q,copy_frame_rate);
    c='\0';
    write(q,c);
  }
  else{
    std::cout << "server_session_receiver: unrecognized command: "
	      << c << ". ignoring" << std::endl;
  }  
  s->send(q);
}

void server_session_status(amjCom::Session,amjCom::Status s){
  if(s.error()){
    std::cout << "FringeTracker: server_session: Error: "
	      << s.errormessage() << std::endl;
  }
  
  if(s.state())
    std::cout << "FringeTracker: server_session: State: "
	      << s.statedescription() << std::endl;
}
