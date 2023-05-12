/* This is the simulator. It contains a simulated delay line and a
   simulated camera. Delay line commands are input and simulated data
   frames are output. The program consists of two threads. The main
   thread is triggered by a timer at regular intervals.


   Main thread
   makes makes phasors or frames using delay in a global
   variable. thread_receiverdelaylines receives delays and places them
   in the global variables */

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
    "--baseline name dl+ dl- nL L0 L1 A S seed",
    "  <string> name - name of the baseline",
    "  <int> dl+ dl- - the baseline delay with be delay(dl+)-delay(dl-)",
    "  where delay() is the total delay on a delayline, including atmosphere",
    "  <int> nL - number of wavelength channels",
    "  <float> L0, L1 - shorter and longe wavelength in micrometers",
    "  <float> A - amplitude of phasors",
    "  <float> S - noise on phasors",
    "  <int> seed - random number seed",
    "--frameinterval <int>",
    "  Number of milliseconds between frames",
    "//--sender-frames",
    "//  A communicator for sending simulated frames.",
    /*=======================================================================*/
  });

#include <mutex>

#include <semaphore.h>
#include <unistd.h>

#include <amjComUDP.H>

#include "../include/DelaylineSimulator.H"
#include "../include/PhasorsSim.H"

void parse_args(int argc, char *argv[]);
void *receiverdelaylines(void *d);

int nDelaylines=6;
int timedelay=5000000;
int frameinterval=10;
std::string receiver_delaylines;
std::string sender_phasors;

std::mutex m;
sem_t framesem;

//std::string sender_frames;

struct PhasorsSim2{
  int dlp;
  int dlm;
  PhasorsSim sim;
};

std::vector<struct PhasorsSim2> phasorssims;

int main(int argc, char *argv[]){
  parse_args(argc,argv);
  sem_init(&framesem,0,0);
  
  DelaylineSimulator delaylinesimulator(nDelaylines,timedelay);
  amjComEndpointUDP sender("",sender_phasors);
  amjPacket packet;
  
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
    sem_wait(&framesem);
    positions=delaylinesimulator.positions();
    
    // get atmosphere delays

    packet << (int)phasorssims.size();
    for(unsigned int i=0;i<phasorssims.size();i++)
      packet << phasorssims[i].sim.phasors(positions[phasorssims[i].dlp]-
					   positions[phasorssims[i].dlm]);
    
    sender.send(packet);
    
    // Wait for timer trigger - in the future alarm will post to the semaphore
    usleep(1000*frameinterval);
    sem_post(&framesem);
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
    else if(strcmp(argv[i],"--baseline")==0){
      phasorssims.push_back({atoi(argv[i+2]),atoi(argv[i+3]),
	    PhasorsSim(argv[i+1],atoi(argv[i+4]),atof(argv[i+5]),
		       atof(argv[i+6]),atof(argv[i+7]),atof(argv[i+8]),
		       atoi(argv[i+9]))});
      i+=9;
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
