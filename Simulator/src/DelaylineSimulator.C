#include "../include/DelaylinSimulator.H"

DelaylineSimulator::DelaylineSimulator(int nDelaylines, int timedelay):
  p(nDelaylines),timedelay(timedelay),now(,{0,0}),realtime(true){}

DelaylineSimulator::DelaylineSimulator(Delays<double initial, int timedelay):
  p(initial),timedelay(timedelay),now({0,0}),realtime(true){}

void DelaylineSimulator::time(struct timespect &t){
  now=t;
  realtime=false;
}

void DelaylineSimulator::movements(Delays<float> &m){
  if(realtime==true)
    clock_gettime(CLOCK_MONOTONIC,&now);
  now.tv_nsec+=timedelay;
  now.tv_sec+=now.tv_nsec/1000000000;
  now.tv_nsec=now.tv_nsec%1000000000;
  Delays<double> latest;
  if(q.size()>0)
    latest=q.back().second;
  else
    latest=p;
  latest+=m;
  q.push({now,latest});
}

Delays<double> DelaylineSimulator::positions(){
  if(realtime==true)
    clock_gettime(CLOCK_MONOTONIC,&now);
  while(true){
    if(q.size()==0)
      break;
    if(q.front().first>now)
      break;
    p=q.front.second();
    q.pop();
  }
  return p;
}

inline bool operator>(struct timespec &lhs, struct timespec &rhs){
  if(lhs.tv_sec>rhs.tv_sec)
    return true;
  if(lhs.tv_sec<rhs.tv_sec)
    return false;
  if(lhs.tv_nsec>rhs.tv_nsec)
    return true;
  return false;
}
