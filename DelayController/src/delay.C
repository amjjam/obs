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
    "--delaylog <string> file to write a running log of delay to",
    "  It contains a header:",
    "  <int32> - the number of delay lines",
    "  Each time a delay update is received a record is written",
    "  <int32> <int32> the time in s and ns in the delay packet",
    "  <int32> <int32> the current time in s and ns",
    "  N of",
    "    <float> the change in the delay",
    "    <double> the accumulated delay (after the change just received",
    "      is applied)"
    /*=====================================================================*/
  });

#include <iostream>
#include <cstring>
#include <mutex>
#include <utility>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <fstream>

#include <amjCom/amjComUDP.H>
#include <amjTime.H>

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

DelaylineInterface<float,double> *delaylineinterface=nullptr;

std::mutex m;

bool debug=false;

// Logging
std::string delaylog;
std::ofstream fplog;
void sigterm_callback(int s){
  if(fplog.is_open())
    fplog.close();
  exit(0);
}

std::vector<pSession> sessions;
void callback_session(pSession){
  
}

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

  amjCom::TCP::Server(":27012",callback_session);
  
  delays.resize(nDelaylines);
  movements.resize(nDelaylines);
  amjTime T,TT;
  
  // Open the delaylog and write the header
  if(delaylog.size()>0){
    fplog.open(delaylog.c_str(),std::ios::binary|std::ios::out);
    fplog.write((char *)&nDelaylines,sizeof(int));
  }
  
  for(int i=0;;i++){
    p.clear();
    r.receive(p);
    T.read(p.read(T.size()));
    p >> movements;
    TT.now();
    if(debug)
      std::cout << movements.size() << std::endl;

    if(delaylineinterface){
      m.lock();
      delaylineinterface->move(movements);
      m.unlock();
    }
    
    if(i%100==0)
      std::cout << "i=" << i << " delays=" << delays <<  std::endl;

    if(fplog.is_open()){
      struct timespec t=T.toTimespec(),tt=TT.toTimespec();
      int32_t a;
      a=t.tv_sec;
      fplog.write((char *)&a,sizeof(int32_t));
      a=t.tv_nsec;
      fplog.write((char *)&a,sizeof(int32_t));
      a=tt.tv_sec;
      fplog.write((char *)&a,sizeof(int32_t));
      a=tt.tv_nsec;
      fplog.write((char *)&a,sizeof(int32_t));
      delays=delaylineinterface->pos();
      for(unsigned int i=0;i<delays.size();i++)
	fplog.write((char *)&delays[i],sizeof(double));
    }
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
    else if(strcmp(argv[i],"--delaylog")==0){
      i++;
      delaylog=argv[i];
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
  amjTime T;
  for(;;){
    m.lock();
    _delays=delaylineinterface->pos();
    m.unlock();
    T.now();
    
    p.clear();
    T.write(p.write(T.size()));
    p << _delays;
    s.send(p);

    if(debug)
      std::cout << _delays << std::endl;
    usleep(sender_display_T*1000);    
  }
  
  return nullptr;
}
