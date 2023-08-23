#ifndef _DELAYLINEINTERFACE_H_
#define _DELAYLINEINTERFACE_H_

#include "../../shared/include/Delays.H"

class DelaylineInterface{
public:
  DelaylineInterface(){};
  ~DelaylineInterface(){};
  virtual void send(const Delays<float> &d)=0;
};

#endif /* _DELAYLINEINTERFACE_H_ */
