#ifndef _PHASORSIM_H_
#define _PHASORSIM_H_

#include <amjFourier.H>
//#include "../../shared/include/Phasors.H"
#include "../../shared/include/Wavelengths.H"

class PhasorsSim{
public:
  PhasorsSim(){}; // Default constructor so it can be used in a vector
  PhasorsSim(const std::string &, int, float, float, float, float, long int);
  //PhasorsSim(Wavelengths &, float, float, int);
  //PhasorsSim(std::vector<float> &, float, float, int);
  virtual ~PhasorsSim();
  Phasors &phasors(float d);
private:
  Wavelengths L;
  Phasors P;
  float A;
  float S;
  long idum;
};


#endif /* _PHASORSIM_H_ */
