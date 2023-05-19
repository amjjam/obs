#include "../include/Sender.H"

void Senders::addsender(std::string &sender, int _interval){
  endpoints.push_back(std::unique_ptr<amjComEndpointUDP>
		      (new amjComEndpointUDP("",sender)));
  interval.push_back(_interval);
  last.push_back({0,0});
}

void Senders::send(amjPacket &p){
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC,&t);
  for(unsigned int i=0;i<endpoints.size();i++){
    if(timediff(t,last[i])>=interval[i]){
      endpoints[i]->send(p);
      last[i]=t;
    }
  }
}

double Senders::timediff(const timespec &t1, const timespec &t2){
  return ((double)t1.tv_sec-(double)t2.tv_sec)*1e3
    +((double)t1.tv_nsec-(double)t2.tv_nsec)/1e6    
}
