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

#include "../include/StateMachine.H"
#include "../include/Baseline2Delayline2Output.H"
#include "../include/DelayFilter.H"

#include "../../shared/include/Timing.H"

#include "../include/Machines.H"
#include "../include/DelayFilter.H"

#include <numeric>
#include <stdexcept>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <utility>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <atomic>

#include <iostream>
#include <fstream>

#include <amjCom/amjComUDP.H>
#include <amjCom/amjComTCP.H>
#include <amjCom/amjComSerialize.H>
#include <amjTime.H>
#include <amjInterferometry.H>

void parse_args(int,char *[]);
void* sample_tracker_stats(void *);
void resizetrackerstats();

std::string receiver_phasors="127.0.0.1:27001";
std::string sender_movements;

int nDelaylines=6;
int stateMachineTSample=10;
int stateMachineTReport=1000;
bool active=false;

// Abbreviate namespace amjInterferometry
namespace ai=amjInterferometry;

// phasors version
static int32_t pversion=-1;
static std::mutex m_pversion;

// Computation loop objects
static std::vector<ai::Phasors<float> > ph;
static std::mutex m_ph;
static PhasorsAverage pha;
static CalcPowerSpectrum cps;
static std::vector<ai::PowerSpectrum<>> ps;
static PowerSpectrumAverage psa;
static PowerSpectrumSmooth pss;
static AnalyzePowerSpectrum aps;
static DelayFilters df;
static StateMachines sm;
static std::vector<float> bm; // baseline movement
static Baseline2Delayline2Output b2d2o;
static std::vector<float> dm; // delayline movement

// Stride function. If it returns true the caller should proceed
// forward. If it returns false the called should return to the start
// of the loop by calling continue
// inline bool stride(int32_t &istride, int32_t nstride){
//   if(++istride<nstride)
//     return false;
//   istride=0;
//   return true;
// }


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
std::mutex m_frame_counter;

// Run state
enum class RunState: uint8_t{Running=1,Stopped=2};
std::atomic<RunState> runstate=RunState::Running;

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
  
  amjComEndpointUDP r(receiver_phasors,"");
  amjComEndpointUDP s("",sender_movements);
  
  amjPacket packet;
  amjPacket packetSNRViewer;
  amjTime T;

  
  
  // Inside main, after declaring the shared variables.
  amjCom::Server server=amjCom::TCP::create_server(server_address,
						   server_session,
						   server_status);
  server->start();
  
  // Open trackerlog and write header information
  if(trackerlog.size()>0){
    fplog.open(trackerlog.c_str(),std::ios::binary|std::ios::out);
    int32_t nBaselines=sm.size();
    fplog.write((char *)&nBaselines,sizeof(int32_t));
    fplog.write((char *)&nDelaylines,sizeof(int));
    for(unsigned int i=0;i<sm.size();i++)
      fplog.write((char *)sm[i].name().c_str(),
		  sm[i].name().size()+1);
    for(int i=0;i<sm[0].nstates();i++)
      fplog.write((char *)sm[0].stateName(i).c_str(),
		  sm[0].stateName(i).size()+1);
  }

  // For cleanly closing the log file on SIGTERM
  signal(SIGTERM,&sigterm_callback);
  
  while(true){
    packet.clear();
    r.receive(packet);

    if(runstate!=RunState::Running)
      continue; // Elsewhere, when runstate is set to not Running, put
		// state machines into state STOP
      
    int32_t new_pversion; // When pversion changes it means phasors layout
    // changed (number of baselines, baseline names, number of wavelengths,
    // wavelength values)
    uint32_t nphasors;
    
    // Load phasors from incoming packet
    {
      std::lock_guard<std::mutex> lock(m_ph);
      memcpy(&new_pversion,packet.read(sizeof(int32_t)),sizeof(int32_t));
      memcpy(&nphasors,packet.read(sizeof(uint32_t)),sizeof(uint32_t));
      ph.resize(nphasors);
      for(size_t j=0;j<ph.size();j++){
	ph[j].read1(packet.read(ph[j].memsize1()));
	ph[j].read2(packet.read(ph[j].memsize2()));
      }
    }
    if(new_pversion!=pversion){
      std::lock_guard<std::mutex> lock(m_pversion);
      pha.config(ph);
      cps.config(ph);
      {
	std::lock_guard<std::mutex> lock=cps.lock();
	ps.resize(ph.size()); // cps lock when writing to ps
      }
      psa.config(ph.size());
      pss.config(ph.size());
      aps.config(ph.size());
      df.config(ph.size());
      sm.config(ph);
      bm.resize(ph.size());
      dm.resize(ph.size()+1); // We need a configuration # delaylines
      // Configure baseline geometry - hard-coded for the time being
      b2d2o.config(Baseline2DelaylineLinearConfig{pversion,{0,1}});

      pversion=new_pversion;
    }
    
    // Print frame count
    if(frame_counter%100==0)
      std::cout << "nframes/100=" << frame_counter/100 << std::endl;
    
    // Average phasors
    {
      std::lock_guard<std::mutex> lock=pha.lock();
      for(size_t j=0;j<pha.size();j++)
	pha[j].add(ph[j]);
      if(!pha.stride())
	continue;
    }
    
    // Calculate power spectra
    {
      std::lock_guard<std::mutex> phalock=pha.lock();
      std::lock_guard<std::mutex> cpslock=cps.lock();
      for(size_t j=0;j<cps.size();j++)
	cps[j](pha[j],ps[j]);
    }
    
    // Average power spectrum (over time dimension)
    {
      std::lock_guard<std::mutex> lock=psa.lock();
      for(size_t j=0;j<psa.size();j++)
	psa[j](ps[j]);
    }
    
    // Smooth power spectrum (over delay dimension)
    {
      std::lock_guard<std::mutex> psalock=psa.lock();
      std::lock_guard<std::mutex> psslock=pss.lock();
      for(size_t j=0;j<pss.size();j++)
	pss[j](psa[j]);
    }
    
    // Analyze power spectra (find peak delay and SNR)
    {
      std::lock_guard<std::mutex> psslock=pss.lock();
      std::lock_guard<std::mutex> apslock=aps.lock();
      for(size_t j=0;j<aps.size();j++)
	aps[j](pss[j]);
    }
    
    // Filter delay and SNR
    {
      std::lock_guard<std::mutex> apslock=aps.lock();
      std::lock_guard<std::mutex> dflock=df.lock();
      for(size_t j=0;j<df.size();j++)
	df[j](aps[j].delay(),aps[j].snr());
      if(!df.stride()) continue;
    }
    
    // Run state machines
    {
      std::lock_guard<std::mutex> dflock=df.lock();
      std::lock_guard<std::mutex> smlock=sm.lock();
      for(size_t j=0;j<sm.size();j++)
	sm[j](df[j].delay(),
	      df[j].snr());

      if(sm.stride())
	continue;
    }
    
    // Retrieve baseline movements from state machine
    // no lock because state machine output is not changed by config(c)
    for(size_t j=0;j<sm.size();j++)
      bm[j]=sm[j].movement();
    
    // Calculate delayline movements from baseline movements
    if(!b2d2o(bm,dm,pversion)) // mutex held in operator() call    
      continue; 
    
    // Send delay line movements to delay line controller
    //Delays<float> delaylinemovements=baseline2delayline.delaylineMovements();
    // packet.clear();
    // T.write(packet.write(T.size()));
    // packet << delaylinemovements;
    // s.send(packet);
    
    // If the log file is open write an entry to it
    if(fplog.is_open()){
      struct timespec t=T.toTimespec();
      int32_t tt=t.tv_sec;
      fplog.write((char *)&tt,sizeof(int32_t));
      tt=t.tv_nsec;
      fplog.write((char *)&tt,sizeof(int32_t));
      for(unsigned int i=0;i<sm.size();i++){
	uint8_t state=sm[i].state();
	fplog.write((char *)&state,sizeof(uint8_t));
	fplog.write((char *)&sm[i].getConfig().snrFound,sizeof(float));
	fplog.write((char *)&sm[i].getConfig().snrLost,sizeof(float));
	float snr=sm[i].snr();
	fplog.write((char *)&snr,sizeof(float));
      }
      for(int i=0;i<nDelaylines;i++)
	fplog.write((char *)&dm[i],sizeof(float));
    } 

    std::lock_guard<std::mutex> lock(m_frame_counter);
    frame_counter++;
  }    
    
  return 0;
}

void parse_args(int argc, char *argv[]){
  help.help(argc,argv);
  
  for(int i=1;i<argc;i++){
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
}


// Compute frame rate
void *sample_tracker_stats(void *dummy){
  uint64_t old_frame_counter;
  {
    std::lock_guard<std::mutex> lock(m_frame_counter);
    old_frame_counter=frame_counter;
  }
  for(;;){
    {
      std::lock_guard<std::mutex> lock(m_frame_counter);
      frame_rate=(uint32_t)(frame_counter-old_frame_counter);
      old_frame_counter=frame_counter;
    }
    sleep(1);
  }  
}

// Compute state machine statistics
void *statemachine_statistics(void *dummy){
  for(;;){
    std::lock_guard<std::mutex> lock=sm.lock();

    //sleep 10 ms
  }
  return nullptr;
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
  amjCom::Packet q;
  char c;
  read(p,c);
  if(c=='G'){
    int32_t version;
    read(p,version);
    // first verify FringeTrackerGUI has the right version
    {
      std::lock_guard<std::mutex> lock(m_pversion);
      if(version!=pversion){
	c='B';
	write(q,c);
	BaselineNameConfig bnc;
	bnc.version=pversion;
	for(auto &iph:ph)
	  bnc.names.push_back(iph.name());
	bnc.write(q.write(bnc.size()));
	s->send(q);
	return;
      }
    }
    // Read configuration packets until end
    uint8_t cc;
    read(p,cc);
    while(cc!='\0'){
      switch(cc){
      case '1':
	uint8_t command;
	read(p,command);
	switch(static_cast<FringeTrackerCommand::CommandType>(command)){
	case FringeTrackerCommand::CommandType::Start:
	  runstate=RunState::Running;
	  break;
	case FringeTrackerCommand::CommandType::Stop:
	  runstate=RunState::Stopped;
	  break;
	default:
	  throw std::runtime_error("FringeTracker: server_session_receive: "
				   "unknown command value: "
				   +std::to_string(static_cast<uint8_t>(command)));
	}
	break;
      case '2':{
	PhasorsAverageConfig c;
	c.read(q.read(c.size()));
	pha.config(c);
	break;
      }
      case '3':{
	CalcPowerSpectrumConfig c;
	c.read(q.read(c.size()));
	cps.config(c);
	break;
      }
      case '4':{
	PowerSpectrumAverageConfig c;
	c.read(q.read(c.size()));
	psa.config(c);
      }
      case '5':{
	PowerSpectrumSmoothingConfig c;
	c.read(q.read(c.size()));
	pss.config(c);
      }
      case '6':{
	AnalyzePowerSpectrumConfig c;
	c.read(q.read(c.size()));
	aps.config(c);
      }
      case '7':{
	DelayFilterConfig c;
	c.read(q.read(c.size()));
	df.config(c);
      }
      case '8':{
	StateMachinesConfig c;
	c.read1(q.read(c.size1()));
	c.read2(q.read(c.size2()));
	sm.config(c);
      }
      case '9':{
	OutputConfig c;
	c.read(q.read(c.size()));
	b2d2o.config(c);
      }
      default:
	throw std::runtime_error(std::format("FringeTracker::server_session_receive:"
					     " unknown section: {} ({:#x}).",c,
					     static_cast<uint8_t>(c)));
      }
      read(p,cc);
    }
    // respond with update packet
    c='U';
    q.clear();
    write(q,c);
    {
      std::lock_guard<std::mutex> lock(m_ph);
      if(ph.size()>0)
	ph[0].time().write(q.write(ph[0].time().size()));
      else{
	amjTime t;
	t.now().write(q.write(t.size()));
      }
    }
    {
      std::lock_guard<std::mutex> lock(m_frame_counter);
      memcpy(q.write(sizeof(uint32_t)),&frame_rate,sizeof(uint32_t));      
    }
    s->send(q);
  }
  else if(c=='P'){ // Send power spectra
    //std::lock_guard<std::mutex> lock(mutex_delaymachines);
    // uint32_t n=delaymachines.size();
    // memcpy(q.write(sizeof(uint32_t)),&n,sizeof(uint32_t));
    // for(unsigned int i=0;i<delaymachines.size();i++)
    //   delaymachines[i].p().write(q.write(delaymachines[i].p().memsize()));
  }
  else if(c=='S'){ // Send SNR values
    // uint32_t version;
    // memcpy(&version,p.read(sizeof(uint32_t)),sizeof(uint32_t)); 
    // //std::lock_guard<std::mutex> lock(mutex_delaymachines);
    // uint32_t n=delaymachines.size();
    // if(version!=version_delaymachines){
    //   c='U';
    //   write(q,c);
    //   write(q,version_delaymachines);
    //   std::vector<std::string> names;
    //   names.reserve(n);
    //   for(const auto &dm: delaymachines)
    // 	names.push_back(dm.name());
    //   write(q,names);
  }
    // c='D';
    // write(q,c);
    // amjTime time;
    // if(n>0){
    //   time=delayMachines[0].time();
    // }
    // else
    //   time.now();
    // time.write(q.write(time.size()));
    // std::vector<float> snrs;
    // snrs.reserve(n);
    // for(auto &dm:delaymachines)
    //   snrs.push_back(dm.snr());
    // write(q,snrs);

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
one section started with one character:
               '\0' - end of packet.
               'd' - complete delay-machine configuration (schema 1)
               'S' - reserved; not implemented

Following '\0': nothing, packet end
Following 'd': uint32_t schema = 1
               uint32_t temporal method (0 none, 1 boxcar, 2 leaky)
               uint32_t temporal width (samples)
               uint32_t delay-axis method (0 none, 1 boxcar, 2 Gaussian)
               float delay-axis width (full width in delay units)
               float peak-exclusion width (full width in delay units)
               uint32_t output method (0 none, 1 boxcar, 2 leaky)
               uint32_t output width (samples)
               Must be followed by '\0'. Applies to all baselines.
The old 'D' format and the unimplemented 'S' section are rejected.
Invalid fields/section terminators are rejected before settings are applied.
A successful update receives the usual 'U' response. That response alone is
not an acceptance acknowledgement; explicit error/readback messages can be
added alongside the GUI sender. Serialize scalars using amjCom, not raw structs.

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
    // uint32_t c_version;
    // read(p,c_version);
    // std::lock_guard<std::mutex> lock(mutex_delayMachines);
    // // If controller has old version send configuration
    // if(c_version!=version_delayMachines){ 
    //   c='C';
    //   write(q,c);
    //   write(q,version_delayMachines);
    //   std::vector<std::string> names;
    //   names.reserve(delayMachines.size());
    //   for(const auto &dm: delayMachines)
    // 	names.push_back(dm.name());
    //   write(q,names);      
    // }
    // else{
      // try{
      //   read(p,c);
      //   if(c=='d'){
      //     apply_delay_machine_config(p);
      //   }
      //   else if(c!='\0'){
      //     throw std::invalid_argument("Unsupported controller section (use 'd', schema 1)");
      //   }
      // }
      // catch(const std::exception& error){
      //   std::cerr << "FringeTracker: configuration rejected: " << error.what()
      //             << std::endl;
      // }
    //}
    // c='U';
    // write(q,c);
    // amjTime T;
    // if(delayMachines.size()>0)
    //   T=delayMachines[0].time();
    // else
    //   T.now();
    // T.write(q.write(T.size()));
    // uint32_t copy_frame_rate;
    // {
    //   std::lock_guard<std::mutex> lock(mutex_frame_counter);
    //   copy_frame_rate=frame_rate;
    // }
    // write(q,copy_frame_rate);
    // c='\0';
    // write(q,c);
  }
  else{
    // std::cout << "server_session_receiver: unrecognized command: "
    // 	      << c << ". ignoring" << std::endl;
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
