#include "../../shared/include/Help.H"

Help help({
    /*=======================================================================*/
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
    "--sender-movements <string>",
    "  Specifies a communicator for sending delay line movements",
    "--delaylines <int>",
    "  Is the number of delaylines. Default is 6.",
    "--delaylines-index {<int>}",
    "  It is assumed that the baselines are in linear order, but the delay",
    "  lines which correspond to endpoints can be out of order. This is the",
    "  index to the delay line and should be one more than the number of",
    "  baselines. Default is 0, 1, 2, ....",
    "--sender-tracker-stats <string> <int> <int>",
    "  Specifies a communicator for sending fringe tracker statistics (first),",
    "  and a sampling interval in ms (second), and a reporting interval in ms",
    "  (third). Defaults for intervals are 5 ms and 1000 ms",
    "--sender-pspec <string> <int>",
    "  Specifies a communicator and a time interval in ms. Sends the delay",
    "  machine average power spectrum via the communicator at the ms interval.",
    "--receiver-tracker-controller <string>",
    "  Specifies a communicator for receiving commands",
    "--sender-tracker-controller <string> <int>",
    "  Specifies a communicator for sending tracker status (first), and an",
    "  interval in ms (second)",
    "--active <int>",
    "  <int> the state machines should begin in the active (1) or inactive (0)",
    "  state. Default is inactive (0).",
    "  The state machines should begin in the inactive state",
    "--trackerlog <string> a file into which the state machine state is logged",
    "  every time it executes. A header is written before the first record",
    "  <int32> the number of baselines (N)",
    "  <int32> the number of delaylines (M)",
    "  N of",
    "    <string> null-terminated strings of each baseline name",
    "  Each time the state machine runs",
    "  <int32> <int32> the time in s and ns in the last frame",
    "  <int32> <int32> the time of recording the state in s and ns",
    "  N of",
    "    <uint8> - the state (0 stop, 1, searching, 2 found, 3 locked, 4 lost)",
    "    <float> - the SNR found level",
    "    <float> - the SNR lost level",
    "    <float> - the current SNR",
    "  M of",
    "    <float> - the movement for each delay line"
    /* ======================================================================*/
  });

#include "../include/FringeTrackerStateMachine.H"
#include "../include/DelayMachineGD.H"
#include "../include/Baseline2DelaylineLinear.H"
#include "../include/FringeTrackerBaselineSpec.H"

#include "../../shared/include/Phasors.H"
#include "../../shared/include/Timing.H"
#include "../../shared/include/PowerSpectrum.H"
#include "../../shared/include/Delays.H"

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
#include <amjTime.H>

void parse_args(int,char *[]);
bool check_dimensions();
void setup_fringe_tracker(std::vector<FringeTrackerBaselineSpec> &);
void* sample_pspec(void *);
void* sample_tracker_stats(void *);
void resizetrackerstats();
void *tracker_controller_receiver(void *);
void *tracker_controller_sender(void *);

std::string receiver_phasors;
std::string sender_movements;
std::string sender_pspec;
unsigned int sender_pspec_interval;
std::string sender_tracker_stats;
unsigned int sender_tracker_stats_sample_interval;
unsigned int sender_tracker_stats_report_interval;
std::string receiver_tracker_controller;
std::string sender_tracker_controller;
unsigned int sender_tracker_controller_interval;

int nDelaylines=6;
int stateMachineTSample=10;
int stateMachineTReport=1000;
bool active=false;

//std::vector<std::string> names;
std::vector<DelayMachineGD> delayMachines;
std::vector<FringeTrackerStateMachine> stateMachines;

PhasorSets phasors;
//std::vector<std::vector<std::complex<float> > > phasors; /* [nBl][nLambda] */
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

Baseline2DelaylineLinear baseline2delayline(0,nDelaylines);

std::mutex m;

int main(int argc, char *argv[]){
  parse_args(argc,argv);

  // Create a thread to sample the power spectra
  pthread_t thread_pspec;
  if(sender_pspec.size()>0)
    if(pthread_create(&thread_pspec,nullptr,sample_pspec,nullptr)!=0){
      perror("could not start thread_pspec");
      exit(EXIT_FAILURE);
    }

  // Create a thread to sample the fringe trackers states and send statistics
  pthread_t thread_tracker_stats;
  if(sender_tracker_stats.size()>0)
    if(pthread_create(&thread_tracker_stats,nullptr,sample_tracker_stats,
		      nullptr)!=0){
      perror("could not start thread_tracker_stats");
      exit(EXIT_FAILURE);
    }

  // Create a thread to receive commands from the Fringe Tracker GUI
  pthread_t thread_tracker_controller_receiver;
  if(receiver_tracker_controller.size()>0)
    if(pthread_create(&thread_tracker_controller_receiver,nullptr,
		      tracker_controller_receiver,
		      nullptr)!=0){
      perror("could not start thread_tracker_controller");
      exit(EXIT_FAILURE);
    }
  
  // Create a thread to send config feedback to the Fringe Tracker GUI
  pthread_t thread_tracker_controller_sender;
  if(sender_tracker_controller.size()>0)
    if(pthread_create(&thread_tracker_controller_sender,nullptr,
		      tracker_controller_sender,nullptr)!=0){
      perror("could not start thread_tracker_controller_sender");
      exit(EXIT_FAILURE);
    }
  
  amjComEndpointUDP r(receiver_phasors,"");
  amjComEndpointUDP s("",sender_movements);
  
  amjPacket packet;
  amjTime T;

  // Open trackerlog
  if(trackerlog.size()>0){
    fplog.open(trackerlog.c_str(),std::ios::binary|std::ios::out);
    int32_t nBaselines=stateMachines.size();
    fplog.write((char *)&nBaselines,sizeof(int32_t));
    fplog.write((char *)&nDelaylines,sizeof(int));
    for(unsigned int i=0;i<stateMachines.size();i++)
      fplog.write((char *)stateMachines[i].name().c_str(),
		  stateMachines[i].name().size()+1);
  }

  // For cleanly closing the log file on SIGTERM
  signal(SIGTERM,&sigterm_callback);
  
  for(int i=0;;i++){
    packet.clear();
    r.receive(packet);
    T.read(packet.read(T.size()));
    packet >> phasors;
    if(i%100==0)
      std::cout << i/100 << std::endl;
    
    // Lock out dimensions changes
    m.lock();
      
    // Check dimensions
    if(!check_dimensions()){
      m.unlock();
      continue;
    }
    
    // Load phasors into delay machines
    for(unsigned int i=0;i<phasors.size();i++)
      delayMachines[i].load(phasors[i]);
    
    // Load delays into state machines
    for(unsigned int i=0;i<delayMachines.size();i++){
      stateMachines[i].loadDelay(delayMachines[i].delay());
      stateMachines[i].advance();
    }
    
    // Load baseline movements into delay line movement calculator
    Delays<float> baselinemovements(stateMachines.size());
    for(unsigned int i=0;i<stateMachines.size();i++)
      baselinemovements[i]=-stateMachines[i].movement();
    //std::cout << baselinemovements;
    //for(unsigned int i=0;i<stateMachines.size();i++)
    //  std::cout << stateMachines[i].stateName() << " ";
    //std::cout << std::endl;
    baseline2delayline.loadBaselineMovements(baselinemovements);
      
    // Send baseline movements to delay line interface
    Delays<float> delaylinemovements=baseline2delayline.delaylineMovements();

    m.unlock();

    //std::cout << delaylinemovements << std::endl;
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
    
  }
    
  return 0;
}


void parse_args(int argc, char *argv[]){
  help.help(argc,argv);
  
  std::vector<FringeTrackerBaselineSpec> baselinespecs;
  
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--baseline")==0){
      baselinespecs.push_back(FringeTrackerBaselineSpec(argv[i+1],atoi(argv[i+2]),atof(argv[i+3]),atof(argv[i+4]),atoi(argv[i+5]),atoi(argv[i+6]),atoi(argv[i+7])));
      i+=7;
    }
    else if(strcmp(argv[i],"--receiver-phasors")==0){
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

  if(baselinespecs.size()>0)
    setup_fringe_tracker(baselinespecs);

}


#include <iostream>
bool check_dimensions(){
  if(phasors.size()!=delayMachines.size()){
    std::cout << "tracker: number of baselines mismatched: delayMachines: "
	      << delayMachines.size() << ", phasors: " << phasors.size()
	      << std::endl;
    return false;
  }
  
  for(unsigned int i=0;i<phasors.size();i++)
    if(phasors[i].size()!=delayMachines[i].size()){
      std::cout << "tracker: number of wavelengths mismatched on baseline "
		<< stateMachines[i].name() << " (" << i << "): delayMachines: "
		<< delayMachines[i].size() << ", phasors: "
		<< phasors[i].size() << std::endl;
      return false;
    }
  
  return true;
}

void setup_fringe_tracker(std::vector<FringeTrackerBaselineSpec> &s){
  int state;
  if(active)
    state=STATE_SEARCH;
  else
    state=STATE_STOP;
  // Next create the new fringe tracker objects
  delayMachines.clear();
  stateMachines.clear();
  for(unsigned int i=0;i<s.size();i++){
    delayMachines.push_back(DelayMachineGD(s[i].name(),s[i].L(),s[i].nOver(),
					   s[i].nIncoherent(),s[i].nSmooth()));
    stateMachines.push_back(FringeTrackerStateMachine(s[i].name(),state));
  }
	
  baseline2delayline=Baseline2DelaylineLinear(s.size(),nDelaylines);
  flag_resizetrackerstats=true;
}

void* sample_pspec(void *dummy){
  amjComEndpointUDP s("",sender_pspec);
  amjPacket packet;
  std::vector<std::string> names;
  std::vector<PowerSpectrum> pspecs;
  for(;;){
    m.lock();
    names.clear();
    pspecs.clear();
    for(unsigned int i=0;i<delayMachines.size();i++){
      names.push_back(delayMachines[i].name());
      pspecs.push_back(delayMachines[i].p());
    }
    m.unlock();
    packet.clear();
    packet << names;
    packet << pspecs;
    s.send(packet);
    usleep(1000*sender_pspec_interval);
  }
}

void *sample_tracker_stats(void *dummy){
  amjComEndpointUDP s("",sender_tracker_stats);
  amjPacket packet;
  unsigned int sender_tracker_time_since_report=0;
  m.lock();
  if(flag_resizetrackerstats)
    resizetrackerstats();
  flag_resizetrackerstats=false;
  m.unlock();
  for(;;){
    m.lock();
    for(unsigned int i=0;i<stats.size();i++)
      stats[i].update(stateMachines[i].state());
    m.unlock();
    if(sender_tracker_time_since_report>=sender_tracker_stats_report_interval){
      packet.clear();
      packet << baselinenames;
      packet << statenames;
      for(unsigned int i=0;i<stats.size();i++){
	packet << stats[i].stats();
	stats[i].clear();
      }
      s.send(packet);
      sender_tracker_time_since_report=0;
    }
    else
      sender_tracker_time_since_report+=sender_tracker_stats_sample_interval;
    usleep(1000*sender_tracker_stats_sample_interval);
  }
}

void resizetrackerstats(){
  unsigned int nbaselines=stateMachines.size();
  unsigned int nstates=stateMachines[0].nstates();
  baselinenames.resize(nbaselines);
  statenames.resize(nstates);
  stats.resize(nbaselines);
  for(unsigned int i=0;i<nbaselines;i++){
    baselinenames[i]=stateMachines[i].name();
    stats[i].resize(nstates);
  }
  for(unsigned int i=0;i<nstates;i++)
    statenames[i]=stateMachines[0].stateName(i);
}

void *tracker_controller_receiver(void *dummy){
  amjComEndpointUDP r(receiver_tracker_controller,"");
  amjPacket packet;
  int n;
  FringeTrackerStateMachineConfig c;
  for(;;){
    std::cout << "tracker_controller_receiver" << std::endl;
    r.receive(packet);
    packet >> n;
    m.lock();
    if((unsigned int)n!=stateMachines.size()){
      m.unlock();
      std::cout << "tracker: tracker controller cmd mismatch: n=" << n
		<< ", stateMachines.size()=" << stateMachines.size()
		<< std::endl;
      continue;
    }
    for(unsigned int i=0;i<stateMachines.size();i++){
      c.read(packet.read(c.size()));
      stateMachines[i].setConfig(c);
    }    
    m.unlock();
  }
}

void *tracker_controller_sender(void *dummy){
  amjComEndpointUDP s("",sender_tracker_controller);
  amjPacket packet;
  std::vector<std::string> names;
  FringeTrackerStateMachineConfig c;
  for(;;){
    std::cout << "tracker_controller_sender" << std::endl;
    packet.clear();
    m.lock();
    names.resize(stateMachines.size());
    for(unsigned int i=0;i<stateMachines.size();i++)
      names[i]=stateMachines[i].name();
    packet << names;
    for(unsigned int i=0;i<stateMachines.size();i++){
      c=stateMachines[i].getConfig();
      c.write(packet.write(c.size()));
    }
    m.unlock();
    s.send(packet);
    usleep(1000000);
  }
}
