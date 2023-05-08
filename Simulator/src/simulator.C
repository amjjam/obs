/* This is the simulator. It contains a simulated delay line and a
   simulated camera. Delay line commands are input and simulated data
   frames are output. Main thread makes makes phasors or frames using delay
   in a global variable. thread_receiverdelaylines receives delays and places
   them in the global variables */

#include "../../shared/include/Help.H"

Help help({
    /*=======================================================================*/
    "Simulator",
    "",
    "--delaylines <int>",
    "  The number of delay lines. Default 6.",
    "--timedelay <int>",
    "  The time delay of the delay lines, in ms",
    "--receiver-delaylines <string>",
    "  A communicator for receiving delay line commands.",
    "--sender-phasors",
    "  A communicator for sending simulated phasors.",
    "--baseline name dl+ dl- nL L0 L1 A S",
    "  <string> name - name of the baseline",
    "  <int> dl+ dl- - the baseline delay with be delay(dl+)-delay(dl-)",
    "  where delay() is the total delay on a delayline, including atmosphere",
    "  <int> nL - number of wavelength channels",
    "  <float> L0, L1 - shorter and longe wavelength in micrometers",
    "  <float> A - amplitude of phasors",
    "  <float> S - noise on phasors",
    "--frameinterval <int>",
    "  Number of milliseconds between frames",
    "//--sender-frames",
    "//  A communicator for sending simulated frames.",
    /*=======================================================================*/
  });

#include <mutex>

#include "../include/DelaylineSimulator.H"

int nDelaylines=6;
int timedelay=5000000;
int frameinterval=10;
std::string receiver_delaylines;
std::string sender_phasors;

std::mutex m;


//std::string sender_frames;

int main(int argc, char *argv[]){
  parse_args(argc,argv);

  DelaylineSimulator delaylinesimulator(nDelaylines,timedelay);

  
  // Create a thread to receive delays
  pthread_t thread_receiverdelaylines;
  if(receiver_delaylines.size()>0)
    if(pthread_create(&thread_receiverdelaylines,nullptr,receiverdelaylines,
		      (void*)&delaylinesimulator)!=0){
      perror("could not start thread_receiverdelaylines");
      exit(EXIT_FAILURE);
    }

  Delays<double> positions;
  for(;;){
    // Wait for timer trigger
    sem_wait(framesem);
    positions=DelayliSimulator.positions();
    
    // get atmosphere delays

    packet << (int)baselines.size();
    for(unsigned int i=0;i<baselines.size();i++){
      // compute phasors
      
      // Write phasors to packet
      
      packet << phasors;
    }
    
    sender.send(packet);
    
    // Wait for timer trigger
  }
  
  return 0;
}


void parse_args(int argc, char *argv[]){
  help.help(argc,argv);

  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--delaylines")==0){
      i++;
      nDelaylines=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--timedelay")==0){
      i++;
      timedelay=atoi(argv[i])*1000000;
    }
    else if(strcmp(argv[i],"--receiver-delaylines")==0){
      i++;
      receiver_delaylines=argv[i];
    }
    else if(strcmp(argv[i],"--sender-phasors")==0){
      i++;
      sender_phasors=argv[i];
    }
    else if(strcmp(argv[i],"--frameinterval")==0){
      i++;
      frameinterval=atoi(argv[i]);
    }
    else{
      std::cout << "unrecognized parameter: " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}


void *receiverdelaylines(void *d){
  amjComEndpointUDP r(receiver_delaylines,"");
  amjPacket packet;
  Delays<float> movements;
  DelaylineSimulator *delaylinesimulator=(DelaylineSimulator*)d;
  for(;;){
    r.receive(packet);
    packet >> movements;
    m.lock();
    delaylinesimulator->movements(movements);
    m.unlock();
  }
  return nullptr;
}
