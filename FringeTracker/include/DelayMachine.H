#ifndef _DELAYMACHINE_H_
#define _DELAYMACHINE_H_

#include <vector>
#include <complex>
#include <utility>

//#include "../../shared/include/Wavelengths.H"
//#include "../../shared/include/Phasors.H"
#include <amjInterferometry.H>

class DelayMachine{
public:
  DelayMachine(){};
  DelayMachine(std::string name):_name(name){};
  virtual ~DelayMachine();
  const std::string &name() const {return _name;}
  //virtual size_t size()=0;
  virtual void load(const amjInterferometry::Phasors<float> &p)=0;
  virtual float delay() const =0;
  virtual float snr() const =0;
protected:
  std::string _name;
};


#endif /* _DELAYMACHINE_H_ */
