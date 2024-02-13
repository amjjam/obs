/*
  This is the simulator. It contains a simulated delay line and a
  simulated camera. Delay line commands are input and simulated data
  frames are output. The program consists of two threads. The main
  thread is triggered by a timer at regular intervals to output frames
  corresponding to the position of the delay lines at that time. The
  second thread listens for delay line commands and updates the dela
  line positions, including an appropriate time delay.
*/

#include "../../shared/include/Help.H"

Help help({
    /*=======================================================================*/
    "Simulator",
    "",
    "--delaylines <int>",
    "  The number of delay lines. Default 6. This should be specified on the",
    "  command line before any instances of --externaldelaymodel",
    "--timedelay <int>",
    "  The time delay of the delay lines, in ms",
    "--receiver-delaylines <string>",
    "  A communicator for receiving delay line commands.",
    "--sender-phasors <string>",
    "  A communicator for sending simulated phasors.",
    "--sender-phasors2 <string> <int>",
    "  A second communicator for sending simulated phasors, together with a",
    "  time-interval in ms. A packet will only be sent via this communicator",
    "  if the time-interval has elapsed since the most recent send",
    "--framesize nL nF - number of pixels in wavelength and in fringe direction",
    "--wavelengthrange L0 L1 - From L0 (lowest L bin) to L1, in microns",
    "--baseline name dl- dl+ nL L0 L1 A S seed",
    "  <string> name - name of the baseline",
    "  <int> dl- dl+ - the baseline delay with be delay(dl+)-delay(dl-)",
    "                  delay line numbers 1 to delaylines",
    "  where delay() is the total delay on a delayline, including atmosphere",
    "  <int> nL - number of wavelength channels",
    "  <float> L0, L1 - shorter and longe wavelength in micrometers",
    "  <float> A - amplitude of phasors",
    "  <float> S - noise on phasors",
    "  <int> seed - random number seed",
    "  the first delay line is numbered 1",
    "--frameinterval <int>",
    "  Number of milliseconds between frames",
    "--externaldelaymodel <int> <string> <other parameters>",
    "  <int> i delay line number, 1 to delaylines",
    "  <string> sin or cos",
    "     <float> A amplitude in micrometers",
    "     <float> T period in seconds",    
    "--sender-frames <string>",
    "  A message queue communicator for sending simulated frames.",
    "  Format: /<name>:nbuffers:buffersize."
    "--sender-frames2 <string> <int>",
    "  A message queue communicator for sending every <int> simulated frame."
    /*=======================================================================*/
  });

#include <mutex>

#include <semaphore.h>
#include <unistd.h>
#include <memory>
#include <stdint.h>
#include <math.h>

#include <amjCom/amjComUDP.H>
#include <amjCom/amjComMQ.H>
#include <amjFourier.H>
#include <amjTime.H>

#include "../include/DelaylineSimulator.H"
#include "../include/PhasorsSim.H"
#include "../include/ExternalDelaySimulator.H"

void parse_args(int argc, char *argv[]);
void *receiverdelaylines(void *d);
void send2(amjPacket &p, amjComEndpointUDP &s);

int nDelaylines=6;
int timedelay=5000000; // In nanoseconds
int frameinterval=10; // in milliseconds
std::string receiver_delaylines;
std::string sender_phasors;
std::string sender_phasors2;
int interval_phasors2;
struct timespec last_phasors2;
std::string sender_frames;
std::string sender_frames2;
int sender_frames2_interval;

std::mutex m;
sem_t framesem;

int nL=256,nF=320;
double L0=1,L1=2.5;

bool debug=false;

//std::string sender_frames;

struct PhasorsSim2{
  int dlm;
  int dlp;
  PhasorsSim sim;
};

std::vector<struct PhasorsSim2> phasorssims;
ExternalDelaySimulator externaldelaysimulator(nDelaylines);

int main(int argc, char *argv[]){

  parse_args(argc,argv);
  sem_init(&framesem,0,1);
  
  DelaylineSimulator delaylinesimulator(nDelaylines,timedelay);
  amjComEndpointUDP sender("",sender_phasors);
  amjComEndpointUDP sender2("",sender_phasors2);
  amjComEndpointMQ senderf("",sender_frames);
  amjComEndpointMQ senderf2("",sender_frames2);
  amjPacket packet,packetf;
  
  // Create a thread to receive delays
  pthread_t thread_receiverdelaylines;
  if(receiver_delaylines.size()>0)
    if(pthread_create(&thread_receiverdelaylines,nullptr,receiverdelaylines,
		      (void*)&delaylinesimulator)!=0){
      perror("could not start thread_receiverdelaylines");
      exit(EXIT_FAILURE);
    }
  
  Delays<double> positions;

  // Initialize the frame simulator
  std::vector<Beam> beams{
    Beam(0,13,[&positions](double t)->double{return positions[0];},
	 [](double L)->double{return 0.02;}),
    Beam(26,13,[&positions](double t)->double{return positions[1];},
	 [](double L)->double{return 0.02;}),
    Beam(78,13,[&positions](double t)->double{return positions[2];},
	 [](double L)->double{return 0.02;})};
  //f.beams(std::vector<Beam>({beam1,beam2,beam3}));

  std::vector<Baseline> baselines{
    Baseline(beams[0],beams[1],
	     [](double L)->std::complex<double>{return 1;}),
    Baseline(beams[1],beams[2],
	     [](double L)->std::complex<double>{return 1;}),
    Baseline(beams[2],beams[0],
	     [](double L)->std::complex<double>{return 1;})};
  //f.baselines(std::vector<Baseline>({baseline1,baseline2,baseline3}));
  
  FourierSim f(beams,baselines,nL,nF);
  
  Frame<double> framed(nL,nF);
  Frame<uint16_t> frameu(nL,nF);
  
  long seed=1;

  double tt[100];

  amjTime T;
  timespec t0,t1,t2,t3;
  clock_gettime(CLOCK_MONOTONIC,&t0);
  for(int i=0;;i++){
    // Wait for timer trigger
    sem_wait(&framesem);
    if(i%100==0){
      clock_gettime(CLOCK_MONOTONIC,&t1);
      double t=(t1.tv_sec-t0.tv_sec)+(double)(t1.tv_nsec-t0.tv_nsec)/1e9;
      double ta=0;
      for(int j=0;j<100;j++)
	ta+=tt[j];
      ta/=100;
      double ts=0;
      for(int j=0;j<100;j++)
	ts+=(tt[j]-ta)*(tt[j]-ta);
      ts/=100;
      ts=sqrt(ts);
      std::cout << i/100 << " " << t << " " << ta << " "
		<< ts << std::endl;
    }
    m.lock();
    positions=delaylinesimulator.positions();
    m.unlock();
    positions+=externaldelaysimulator.delays();

    if(debug)
      std::cout << positions << std::endl;
    
    //packet.clear();
    //packet << (int)phasorssims.size();
    //for(unsigned int i=0;i<phasorssims.size();i++)
    //  packet << phasorssims[i].sim.phasors(positions[phasorssims[i].dlp]-
    //					   positions[phasorssims[i].dlm]);
    
  //sender.send(packet);
  // send2(packet,sender2);
    
    if(sender_frames.size()>0&&(i%1)==0){
      if(debug)
	std::cout << "frame" << std::endl;
      clock_gettime(CLOCK_MONOTONIC,&t2);
      f.frame(0,framed);
      poisson<double,uint16_t>(framed,frameu,seed);
      clock_gettime(CLOCK_MONOTONIC,&t3);
      tt[i%100]=(t3.tv_sec-t2.tv_sec)+(double)(t3.tv_nsec-t2.tv_nsec)/1e9;
      packetf.clear();
      T.write(packet.write(T.size()));
      packetf << frameu;
      senderf.send(packetf);
      if(sender_frames2.size()>0&&(i%sender_frames2_interval)==0){
	if(debug)
	  std::cout << "frame2" << std::endl;
	senderf2.send(packetf);
      }
    }

    // Wait for timer trigger - in the future alarm will post to the semaphore
    //usleep(1000*frameinterval);
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
    else if(strcmp(argv[i],"--sender-phasors2")==0){
      i++;
      sender_phasors2=argv[i];
      i++;
      interval_phasors2=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--framesize")==0){
      i++;
      nL=atoi(argv[i]);
      i++;
      nF=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--wavelengthrange")==0){
      i++;
      L0=atof(argv[i]);
      i++;
      L1=atof(argv[i]);
    }
    else if(strcmp(argv[i],"--frameinterval")==0){
      i++;
      frameinterval=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--baseline")==0){
      phasorssims.push_back({atoi(argv[i+2])-1,atoi(argv[i+3])-1,
	    PhasorsSim(argv[i+1],atoi(argv[i+4]),atof(argv[i+5]),
		       atof(argv[i+6]),atof(argv[i+7]),atof(argv[i+8]),
		       atoi(argv[i+9]))});
      i+=9;
    }
    else if(strcmp(argv[i],"--externaldelaymodel")==0){
      int index,function;
      i++;
      index=atoi(argv[i])-1;
      i++;
      if(strcmp(argv[i],"sin")==0)
	function=EXTERNALDELAYSIMULATOR_SIN;
      else if(strcmp(argv[i],"cos")==0)
	function=EXTERNALDELAYSIMULATOR_COS;
      else if(strcmp(argv[i],"square")==0)
	function=EXTERNALDELAYSIMULATOR_SQUARE;
      else{
	std::cout << "Simulator: unrecognized external delay model: "
		  << argv[i] << std::endl;
	exit(EXIT_FAILURE);
      }
      std::vector<double> params;
      i++;
      params.push_back(atof(argv[i]));
      i++;
      params.push_back(atof(argv[i]));
      externaldelaysimulator.function(index,function,params);
    }
    else if(strcmp(argv[i],"--sender-frames")==0){
      i++;
      sender_frames=argv[i];
    }
    else if(strcmp(argv[i],"--sender-frames2")==0){
      i++;
      sender_frames2=argv[i];
      i++;
      sender_frames2_interval=atoi(argv[i]);
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
    packet.clear();
    r.receive(packet);
    packet >> movements;
    m.lock();
    delaylinesimulator->movements(movements);
    m.unlock();
  }
  return nullptr;
}

void send2(amjPacket &p, amjComEndpointUDP &sender2){
  if(sender_phasors2.size()>0){
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC,&now);
    if(((double)now.tv_sec-(double)last_phasors2.tv_sec)*1e3
       +((double)now.tv_nsec-(double)last_phasors2.tv_nsec)/1e6
       >interval_phasors2){
      sender2.send(p);
      last_phasors2=now;
    }
  }
}


