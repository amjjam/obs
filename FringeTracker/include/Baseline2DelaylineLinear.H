#ifndef _BASELINE2DELAYLINELINEAR_H_
#define _BASELINE2DELAYLINELINEAR_H_

#include "Baseline2Delayline.H"

// Combines baseline delays to make delayline delays. It is assumed that
// the baseline delays combine in order, and that the delaylines in that
// order are addressed by index.
// So set D[index[0]]=0, D[index[i]]=D[index[i-1]]+B[i-1]
// Then subtract the average delay
class Baseline2DelaylineLinear: public Baseline2Delayline{
public:
  Baseline2DelaylineLinear(int nB=0, int nD=0, std::vector<int> index={});
  virtual ~Baseline2DelaylineLinear(){};
  virtual Delays<float> delaylineMovements();
private:
  std::vector<int> index; /* index from delay into delaylines */
};




#endif /* _BASELINE2DELAYLINELINEAR_H_ */
