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
    "  It is assumed that the baselines are in linear order, but the delay lines",
    "  which correspond to endpoints can be out of order. This is the index to",
    "  the delay line and should be one more than the number of baselines. Default",
    "  it 0, 1, 2, ....",
    "--state-machine-stats <int> <int>",
    "  Controls how often fringe tracker state machine statistics are collected",
    "  and reported. The first number is the number of ms between samples. The",
    "  second is the number of ms between reports. Defaults are 10 1000.",
    "--sender-pspec <string> <int>",
    "  Specifies a communicator and a time interval in ms. Sends the delay",
    "  machine average power spectrum via the communicator at the ms interval.",
    "  Should only be specified once on the command line. The format of the",
    "  packet is",
    "  m <int> - number of power spectra",
    "  Then follows m blocks:"
    "  k <int> - size of following string",
    "  s <string> - k characters",
    "  n <int> - number of coordinates"
    "  x <float>[n] - n floats for the x coordinates"
    "  y <float>[n] - n floats for the y values"    
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

#include <iostream>

#include <amjComUDP.H>

void parse_args(int,char *[]);
bool check_dimensions();
void setup_fringe_tracker(std::vector<FringeTrackerBaselineSpec> &);
void* sample_pspec(void *);

std::string receiver_phasors;
std::string sender_movements;
std::string sender_pspec;
unsigned int sender_pspec_interval;

int nDelaylines=6;
int stateMachineTSample=10;
int stateMachineTReport=1000;

std::vector<std::string> names;
std::vector<DelayMachineGD> delayMachines;
std::vector<FringeTrackerStateMachine> stateMachines;

PhasorSets phasors;
//std::vector<std::vector<std::complex<float> > > phasors; /* [nBl][nLambda] */
std::vector<std::pair<float,float> > baselinemovements;

int nD=6;
Baseline2DelaylineLinear baseline2delayline(0,nD);

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
  
  //sem_init(&s_phasors,0,0);
  // Create a listening thread for:
  // 1: configuration (baselines, wavelengths)
  // 2: new phasors
  // 3: commands for updating parameters of delay machines and state machines

  // Create a thread for sampling state information for diagnostic displays


  
  amjComEndpointUDP r(receiver_phasors,"");
  amjComEndpointUDP s("",sender_movements);
  
  amjPacket packet;
  
  for(int i=0;;i++){
    //sem_wait(&s_phasors); // Wait for new phasors to arrive

    packet.clear();
    r.receive(packet);
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
      baselinemovements[i]=stateMachines[i].movement();
    std::cout << baselinemovements;
    for(unsigned int i=0;i<stateMachines.size();i++)
      std::cout << stateMachines[i].stateName() << " ";
    std::cout << std::endl;
    baseline2delayline.loadBaselineMovements(baselinemovements);
      
    // Send baseline movements to delay line interface
    Delays<float> delaylinemovements=baseline2delayline.delaylineMovements();

    m.unlock();

    std::cout << delaylinemovements << std::endl;
    packet.clear();
    packet << delaylinemovements;
    s.send(packet);
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
    else if(strcmp(argv[i],"--sender-pspec")==0){
      i++;
      sender_pspec=argv[i];
      i++;
      sender_pspec_interval=atoi(argv[i]);
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
		<< names[i] << " (" << i << "): delayMachines: "
		<< delayMachines[i].size() << ", phasors: "
		<< phasors[i].size() << std::endl;
      return false;
    }
  
  return true;
}

void setup_fringe_tracker(std::vector<FringeTrackerBaselineSpec> &s){
  // Next create the new fringe tracker objects
  delayMachines.clear();
  stateMachines.clear();
  for(unsigned int i=0;i<s.size();i++){
    delayMachines.push_back(DelayMachineGD(s[i].L(),s[i].nOver(),
					   s[i].nIncoherent(),s[i].nSmooth()));
    stateMachines.push_back(FringeTrackerStateMachine(s[i].name()));
  }
	
  baseline2delayline=Baseline2DelaylineLinear(s.size(),nD);
}

void* sample_pspec(void *dummy){
  amjComEndpointUDP s("",sender_pspec);
  amjPacket packet;
  std::vector<PowerSpectrum> pspecs;
  for(;;){
    m.lock();
    pspecs.clear();
    for(unsigned int i=0;i<delayMachines.size();i++)
      pspecs.push_back(delayMachines[i].p());
    m.unlock();
    packet.clear();
    packet << pspecs;
    s.send(packet);
    usleep(1000*sender_pspec_interval);
  }
}

