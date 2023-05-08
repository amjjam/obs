#ifndef _DELAYMACHINE_H_
#define _DELAYMACHINE_H_

#include <vector>
#include <complex>
#include <utility>

#include "../../shared/include/Wavelengths.H"
#include "../../shared/include/Phasors.H"

class DelayMachine{
public:
  DelayMachine(){};
  DelayMachine(const Wavelengths &L):L(L){};
  virtual ~DelayMachine()=0;
  long unsigned int size() const {return L.size();}
  virtual int load(Phasors &p)=0;
  virtual std::pair<float,float> delay()=0;
protected:
  Wavelengths L;
};


#endif /* _DELAYMACHINE_H_ */
