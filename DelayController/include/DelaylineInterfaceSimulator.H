#ifndef _DELAYLINEINTERFACESIMULATOR_H_
#define _DELAYLINEINTERFACESIMULATOR_H_

#include <amjCom/amjComUDP.H>

#include "DelaylineInterface.H"

class DelaylineInterfaceSimulator:public DelaylineInterface<float,double>{
public:
  DelaylineInterfaceSimulator(unsigned int ndelaylines,std::string sender):
    ndelaylines(ndelaylines),plocal(ndelaylines),pdl(ndelaylines),
    s("",sender){};
  ~DelaylineInterfaceSimulator(){};
  virtual void move(const Delays<float> &);
  virtual void moveto(const Delays<double> &);
  virtual void setto(const Delays<double> &,int i=0);
  virtual int nsets(){return 2;};
  virtual Delays<double> pos(int i=0);
private:
  unsigned int ndelaylines;
  Delays<double> plocal;
  Delays<double> pdl;
  amjComEndpointUDP s;
  amjPacket p;
};

#endif /* _DELAYLINEINTERFACESIMULATOR_H_ */
