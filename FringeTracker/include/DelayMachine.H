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
  DelayMachine(std::string name, const Wavelengths &L):_name(name),L(L){};
  virtual ~DelayMachine()=0;
  std::string name() const {return _name;}
  long unsigned int size() const {return L.size();}
  virtual int load(Phasors &p)=0;
  virtual std::pair<float,float> delay()=0;
protected:
  std::string _name;
  Wavelengths L;
};


#endif /* _DELAYMACHINE_H_ */
