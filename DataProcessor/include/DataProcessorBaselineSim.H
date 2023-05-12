#ifndef _DATAPROCESSORBASELINESIM_H_
#define _DATAPROCESSORBASELINESIM_H_

#include <string>

#include "../../shared/include/Phasors.H"

class DataProcessorBaselineSim{
public:
  DataProcessorBaselineSim(std::string &name, int n, float L0, float L1,
			   float A, float S, float d=0, long idum=1);
  virtual ~DataProcessorBaselineSim(){}
  Phasors &makePhasors(float d=-1e31);
private:
  std::string name;
  int n;
  float L0;
  float L1;
  float A;
  float S;
  float d;
  float beta0;
  float beta1;
  float dbeta;
  Phasors phasors;
  long idum;
};

float ran1(long *idum);
float gasdev(long *idum);

#endif /* _DATAPROCESSORBASELINESIM_H_ */
