#ifndef _FRINGETRACKER2DELAYLINEINTERFACE_H_
#define _FRINGETRACKER2DELAYLINEINTERFACE_H_

#include <string>

class FringeTracker2DelaylineInterface{
public:
  FringeTracker2DelaylineInterface(int nD, std::string delaylineinterface);
  ~FringeTracker2DelaylineInterface();
  void load(std::vector<double> movement);
private:
  int nD;

};

#endif /* _FRINGETRACKER2DELAYLINEINTERFACE_H_ */
