#include "../include/ExternalDelaySimulator.H"

#include <cmath>

ExternalDelaySimulator::ExternalDelaySimulator(int n, struct timespec _start){
  resize(n);
  if(_start==(struct timespec){0,0})
    clock_gettime(CLOCK_MONOTONIC,&start);
  else
    start=_start;
};

void ExternalDelaySimulator::resize(int n){
  _function.resize(n,EXTERNALDELAYSIMULATOR_ZERO);
  _atm.resize(n,nullptr);
  params.resize(n,{});
  _delays.resize(n);
}

void ExternalDelaySimulator::function(int i, int f, std::vector<double> &p){
  _function[i]=f;
  params[i]=p;
  if(f==EXTERNALDELAYSIMULATOR_ATM){
    std::cout << "atm" << std::endl;
    amjRandom *random=new amjRandom(-(i+1));
    _atm[i]=new amjAtmosphere(random,params[i][0],params[i][1],100000,
			      AMJATMOSPHERE_MODE_ZERO);
  }
}

Delays<double> &ExternalDelaySimulator::delays(const struct timespec &t){
  double T=timediff(t);
  for(unsigned int i=0;i<_function.size();i++)
    if(_function[i]==EXTERNALDELAYSIMULATOR_ZERO)
      _delays[i]=0;
    else if(_function[i]==EXTERNALDELAYSIMULATOR_SIN)
      _delays[i]=params[i][0]*sin(2*M_PI*T/params[i][1]);
    else if(_function[i]==EXTERNALDELAYSIMULATOR_COS)
      _delays[i]=params[i][0]*cos(2*M_PI*T/params[i][1]);
    else if(_function[i]==EXTERNALDELAYSIMULATOR_SQUARE)
      _delays[i]=params[i][0]*(((int)(2*T/params[i][1])%2)*2-1);
    else if(_function[i]==EXTERNALDELAYSIMULATOR_ATM){
      _delays[i]=_atm[i]->get(T*1000);
      //std::cout << "atm " << i << " " << _delays[i] << " " << T << std::endl;
    }
    else{
      std::cout << "ExternalDelaySimulator: unrecognized function: "
		<< _function[i] << std::endl;
      exit(EXIT_FAILURE);
    }
  return _delays;
}

double ExternalDelaySimulator::timediff(struct timespec t){
  if(t==(struct timespec){0,0})
    clock_gettime(CLOCK_MONOTONIC,&t);
  return (double)(t.tv_sec-start.tv_sec)
    +((double)(t.tv_nsec-start.tv_nsec)/1e9);
}

inline bool operator==(const struct timespec &t1, const struct timespec &t2){
  if(t1.tv_sec==t2.tv_sec&&t1.tv_nsec==t2.tv_nsec)
    return true;
  return false;
}
