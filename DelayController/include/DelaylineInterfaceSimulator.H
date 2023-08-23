#ifndef _DELAYLINEINTERFACESIMULATOR_H_
#define _DELAYLINEINTERFACESIMULATOR_H_

#include <amjComUDP.H>

#include "DelaylineInterface.H"

class DelaylineInterfaceSimulator:public DelaylineInterface{
public:
  DelaylineInterfaceSimulator(unsigned int ndelaylines,std::string sender):
    ndelaylines(ndelaylines),s("",sender){};
  ~DelaylineInterfaceSimulator(){};
  virtual void send(const Delays<float> &);
private:
  unsigned int ndelaylines;
  amjComEndpointUDP s;
  amjPacket p;
};

#endif /* _DELAYLINEINTERFACESIMULATOR_H_ */
