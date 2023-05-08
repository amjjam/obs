#ifndef _BASELINE2DELAYLINE_H_
#define _BASELINE2DELAYLINE_H_

#include <vector>

#include "../../shared/include/Delays.H"

class Baseline2Delayline{
public:
  Baseline2Delayline(int nB, int nD);
  virtual ~Baseline2Delayline()=0;
  void loadBaselineMovements(Delays<float> &b);
  virtual Delays<float> delaylineMovements()=0;
protected:
  Delays<float> B;
  Delays<float> D;
};

#endif /* _BASELINE2DELAYLINE_H_ */
