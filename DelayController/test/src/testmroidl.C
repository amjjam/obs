/*=============================================================================
  testmroidl --server <string> --client <string> --message <parameters>

  --server <ip>:<port> <ip>:<port> - IP address and port of the
    server and client, respectively
  --client <ip>:<port> <ip>:<port> - IP address and port of the
    client and server, respectively
  --message <numbers> -- - list of numbers terminated by -- which is
    movement for each delay line 1-10.

  In server mode program waits for a message and responds with
  position. In client mode program sends a message, wait for one back,
  then exits.
  ============================================================================*/

#include "../../include/DelaylineInterfaceMROI.H"
#include "../../../shared/include/Delays.H"

#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>

bool isServer;
double message[MAX_DLS]={0,0,0,0,0,0,0,0,0,0};
std::string me,peer;

void parse_args(int argc, char *argv[]);

int main(int argc, char *argv[]){
  parse_args(argc,argv);

  if(isServer){
    amjComEndpointUDP c(me,peer);
    amjPacket p;
    c.receive(p);
    FringeOffset offset[MAX_DLS];
    memcpy(offset,p.read(sizeof(FringeOffset)*MAX_DLS),
	   sizeof(FringeOffset)*MAX_DLS);
    DelayLinePosition pos[MAX_DLS];
    std::cout << "received:";
    for(int i=0;i<10;i++){
      pos[i].position[SAMPLES_PER_CHUNK-1]=offset[i].offset;
      std::cout << " " << offset[i].offset*1e6;
    }
    std::cout << std::endl;
    p.start();
    memcpy(p.write(sizeof(DelayLinePosition)*MAX_DLS),pos,
	   sizeof(DelayLinePosition)*MAX_DLS);
    c.send(p);
    std::cout << "position sent" << std::endl;
  }
  else{
    DelaylineInterfaceMROI dl(MAX_DLS,peer,me);
    Delays<double> ds(MAX_DLS),d0,d1;
    d0=dl.pos(0);
    d1=dl.pos(1);
    std::cout << "before move:" << std::endl;
    std::cout << "plocal=" << d0 << std::endl;
    std::cout << "pdl=" << d1 << std::endl;
    for(int i=0;i<MAX_DLS;i++)
      ds[i]=message[i];
    dl.move(ds);
    std::cout << "moving: " << ds << std::endl;
    std::cout << "waiting 1 s to read positions" << std::endl;
    sleep(1);
    d0=dl.pos(0);
    d1=dl.pos(1);
    std::cout << "after move:" << std::endl;
    std::cout << "plocal=" << d0 << std::endl;
    std::cout << "pdl=" << d1 << std::endl;
  }

  return 0;
}

void parse_args(int argc, char *argv[]){
  for(int i=1;i<argc;i++)
    if(strcmp(argv[i],"--server")==0||strcmp(argv[i],"--client")==0){
      if(strcmp(argv[i],"--server")==0)
	isServer=true;
      i++;
      me=argv[i];
      i++;
      peer=argv[i];
    }
    else if(strcmp(argv[i],"--message")==0){
      i++;
      for(int j=0;j<10;j++,i++)
	if(strcmp(argv[i],"--")==0)
	  break;
	else
	  message[j]=atof(argv[i]);
    }
}
