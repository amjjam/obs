#ifndef _DELAYLINEINTERFACE_H_
#define _DELAYLINEINTERFACE_H_

#include "../../shared/include/Delays.H"

template<class moveType, class posType>
class DelaylineInterface{
public:
  DelaylineInterface(){};
  virtual ~DelaylineInterface(){};
  virtual void move(const Delays<moveType> &)=0;
  virtual void moveto(const Delays<posType> &)=0;
  virtual void setto(const Delays<posType> &,int i=0)=0;
  virtual int nsets(){return 1;}; // returns number of sets
  virtual Delays<posType> pos(int i=0)=0; // i in case more than one set 
};

#endif /* _DELAYLINEINTERFACE_H_ */
