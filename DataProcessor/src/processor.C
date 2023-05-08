/*============================================================================
  procssor [--receiver <string>] [--tracker <string>]
  
  --receiver-frames <string>
    Specifies a communicator for listening for data frames
  --sender-tracker <string>
    Specifies a communicator to the tracker
  --sender-phasorviewer <string>
    Specifies a communicator to the phasor viewer
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
  --fourier-baseline
    TBD specifies the information needed to compute phasors from a FOURIER
    frame
  ===========================================================================*/

#include <cstring>
#include <iostream>
#include <complex>
#include <unistd.h>
#include <cstdlib>
#include <time.h>

#include <amjComUDP.H>

#include "../../shared/include/Phasors.H"
#include "../include/DataProcessorBaselineSim.H"

void parse_args(int argc, char *argv[]);
int read_argv_baseline(char *argv[]);

std::string receiver_frames;
std::string sender_tracker;
std::string sender_phasorviewer;

PhasorSets phasorsets;
std::vector<DataProcessorBaselineSim> sim;
time_t t0=0;
time_t t1;

int main(int argc, char *argv[]){
  parse_args(argc,argv);
  
  amjComEndpointUDP r(receiver_frames,"");
  amjComEndpointUDP s("",sender_tracker);
  amjComEndpointUDP p("",sender_phasorviewer);
  
  amjPacket f;
  amjPacket packet;
  
  phasorsets.resize(sim.size());

  for(int i=0;;i++){
    // Wait for frames
    usleep(10000);
    
    // Process frames
    for(unsigned int j=0;j<phasorsets.size();j++)
      phasorsets[j]=sim[j].makePhasorSet();

    packet.reset();
    packet << phasorsets;
    
    // Send results to tracker
    s.send(packet);
    if(i%100==0)
      std::cout << i/100 << std::endl;

    // Send results to phasor viewer
    t1=time(NULL);
    if(t1>t0){
      t0=t1;
      if(sender_phasorviewer.size()>0){
	packet.reset();
	p.send(packet);
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
    else if(strcmp(argv[i],"--baseline")==0){
      i+=read_argv_baseline(argv+i+1);
    }
    else{
      std::cout << "unknown parameter: " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

int read_argv_baseline(char *argv[]){
  std::string name=argv[0];
  int n=atoi(argv[1]);
  float L0=atof(argv[2]);
  float L1=atof(argv[3]);
  float A=atof(argv[4]);
  float S=atof(argv[5]);
  float d=atof(argv[6]);
  sim.push_back(DataProcessorBaselineSim(name,n,L0,L1,A,S,d));
  
  return 7;
}

