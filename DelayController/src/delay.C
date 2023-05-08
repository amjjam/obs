#include "../../shared/include/Help.H"

Help help({
    /*=====================================================================*/
    "DelayController",
    "",
    "--delaylines <int>",
    "  The number of delay lines",
    "--receiver-movements",
    "  Defines communicator for receiving delayline movements from the ",
    "  fringe tracker",
    "--sender-display <string> <int>",
    "  Defines a communicator and time interval for sending delayline ",
    "  positions to a display. The first parameter is the communicator,",
    "  and the second is the time interval in ms."
    "--sender-delaylines <string>",
    "  A communicator for connection to the delay lines."
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

void parse_args(int argc, char *argv[]);
void *senderdisplay(void *);

std::string receiver_movements;
std::string sender_display;
int sender_display_T;

int nDelaylines=6;
Delays<double> delays;
Delays<double> movements;

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
    
    if(i%100==0)
      std::cout << i/100 << " " << delays <<  std::endl;
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
      sener_delaylines
    else{
      std::cout << "unknown parameter: " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void *senderdisplay(void *dummy){

  Delays<double> _delays;
  
  for(;;){
    m.lock();
    _delays=delays;
    m.unlock();

    std::cout << _delays << std::endl;
    usleep(sender_display_T*1000);    
  }

  return nullptr;
}
