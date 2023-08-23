#include "../../shared/include/Help.H"

Help help({
    /*=====================================================================*/
    "DelayController",
    "",
    "--ndelaylines <int>",
    "  The number of delay lines. The default value is 6.",
    "--receiver-movements",
    "  Defines communicator for receiving delayline movements from the ",
    "  fringe tracker",
    "--sender-display <string> <int>",
    "  Defines a communicator and time interval for sending delayline ",
    "  positions to a display. The first parameter is the communicator,",
    "  and the second is the time interval in ms."
    "--sender-delaylines <string> <parameters>",
    "  Definition of a delayline interface. The string is the format",
    "  (sim/mroi/npoi). The rest are interface-specfic parameters",
    "  sim <int> <string>",
    "  <int> is the number of delaylines, <string> is the communicator",
    /*=====================================================================*/
  });

#include <iostream>
#include <cstring>
#include <mutex>
#include <utility>
#include <pthread.h>
#include <unistd.h>

#include <amjComUDP.H>

#include "../../shared/include/Delays.H"

#include "../include/DelaylineInterfaceSimulator.H"

void parse_args(int argc, char *argv[]);
void *senderdisplay(void *);

std::string receiver_movements;
std::string sender_display;
int sender_display_T;
std::string sender_delaylines;

int nDelaylines=6;
Delays<double> delays;
Delays<float> movements;

DelaylineInterface *delaylineinterface=nullptr;

std::mutex m;

int main(int argc, char *argv[]){
  parse_args(argc,argv);

  // Create a thread to sample the delays
  pthread_t thread_senderdisplay;
  if(sender_display.size()>0)
    if(pthread_create(&thread_senderdisplay,nullptr,senderdisplay,nullptr)!=0){
      perror("could not start thread_senderdisplay");
      exit(EXIT_FAILURE);
    }
  
  amjComEndpointUDP r(receiver_movements,"");
  amjComEndpointUDP s("",sender_delaylines);
  amjPacket p;
  delays.resize(nDelaylines);
  movements.resize(nDelaylines);
  
  for(int i=0;;i++){
    p.clear();
    r.receive(p);
    p >> movements;
    std::cout << movements.size() << std::endl;
    m.lock();
    delays+=movements;
    m.unlock();

    if(delaylineinterface)
      delaylineinterface->send(movements);
    
    if(i%100==0)
      std::cout << i/100 << " " << delays <<  std::endl;
  }
  
  return 0;
}


void parse_args(int argc, char *argv[]){
  help.help(argc,argv);
  
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--ndelaylines")==0){
      i++;
      nDelaylines=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--receiver-movements")==0){
      i++;
      receiver_movements=argv[i];
    }
    else if(strcmp(argv[i],"--sender-display")==0){
      i++;
      sender_display=argv[i];
      i++;
      sender_display_T=atoi(argv[i]);
    }
    else if(strcmp(argv[i],"--sender-delaylines")==0){
      i++;
      if(strcmp(argv[i],"sim")==0){
	delaylineinterface=new DelaylineInterfaceSimulator(atoi(argv[i+1]),
							   argv[i+2]);
	i+=2;
      }
      else{
	std::cout << "delayline type unknown or not defined: " << argv[i]
		  << std::endl;
	exit(EXIT_FAILURE);
      }
    }
    else{
      std::cout << "unknown parameter: " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void *senderdisplay(void *dummy){
  amjComEndpointUDP s("",sender_display);
  amjPacket p;
  Delays<double> _delays;
  
  for(;;){
    m.lock();
    _delays=delays;
    m.unlock();

    p.clear();
    p << _delays;
    s.send(p);
    
    std::cout << _delays << std::endl;
    usleep(sender_display_T*1000);    
  }
  
  return nullptr;
}
