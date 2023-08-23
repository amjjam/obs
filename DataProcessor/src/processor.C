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
  --fourier-baseline p
    p the period in pixels of the shortest wavelength channel
    (specify once for each baseline)
  ===========================================================================*/

#include <cstring>
#include <iostream>
#include <complex>
#include <unistd.h>
#include <cstdlib>
#include <time.h>

#include <amjComUDP.H>
#include <amjComMQ.H>

#include <amjFourier.H>

//#include "../../shared/include/Phasors.H"
//#include "../include/DataProcessorBaselineSim.H"

void parse_args(int argc, char *argv[]);
int read_argv_baseline(char *argv[]);

std::string receiver_frames;
std::string sender_tracker;
std::string sender_phasorviewer;

//std::vector<DataProcessorBaselineSim> sim;
time_t t0=0;
time_t t1;

int main(int argc, char *argv[]){
  parse_args(argc,argv);
  
  amjComEndpointMQ r(receiver_frames,"");
  amjComEndpointUDP s("",sender_tracker);
  amjComEndpointUDP p("",sender_phasorviewer);
  
  amjPacket fpacket;
  amjPacket ppacket;

  FourierCompute compute;
  PhasorSets phasors;

  Frame<uint16_t> frame(256,320);
  
  for(int i=0;;i++){
    // Wait for packet
    r.receive(fpacket);

    std::cout << "received" << std::endl;
    
    // Read frame
    fpacket >> frame;
    
    // Process frame
    compute.phasors(frame,{4,8,12},phasors);

    // Write phasors into packet
    ppacket.clear();
    ppacket << phasors;
    
    // Send results to tracker
    s.send(ppacket);
    if(i%100==0)
      std::cout << i/100 << std::endl;
    
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

