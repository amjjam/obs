#ifndef _BASELINE2DELAYLINELINEAR_H_
#define _BASELINE2DELAYLINELINEAR_H_

#include "Baseline2Delayline.H"
#include <numeric>

// Combines baseline delays to make delayline delays. It is assumed that
// the baseline delays combine in order, and that the delaylines in that
// order are addressed by index.
// So set D[index[0]]=0, D[index[i]]=D[index[i-1]]+B[i-1]
// Then subtract the average delay
template<typename T=float>
class Baseline2DelaylineLinear: public Baseline2Delayline<T>{
public:
  Baseline2DelaylineLinear(size_t nB=0, size_t nD=0,
			   std::vector<size_t> index={});
  virtual ~Baseline2DelaylineLinear(){};
  void operator()(const std::vector<T> &, std::vector<T> &) override;
  // It is the responsibility of the caller of operator() to ensure
  // that the elements of d which are not assigned in the function are
  // set to appropriate values, for example 0
private:
  size_t nB,nD;
  std::vector<size_t> index; /* index from delay into delaylines */
};

template<typename T>
Baseline2DelaylineLinear<T>::Baseline2DelaylineLinear(size_t nB, size_t nD,
						      std::vector<size_t> index)
  :nB(nB),nD(nD),index(index){
  if(index.size()==0){
    index.resize(nB+1);
    std::iota(index.begin(),index.end(),0);
  }
  else if(index.size()!=nB+1)
    throw std::runtime_error("Baseline2DelaylineLinear: index.size()="
			     +std::to_string(index.size())
			     +", should be nB+1, and nB="
			     +std::to_string(nB)+".");
}

template<typename T>
void Baseline2DelaylineLinear<T>::operator()(const std::vector<T> &b,
					     std::vector<T> &d){
  d[index[0]]=0;
  for(size_t i=1;i<index.size();i++)
    d[index[i]]=d[index[i-1]]+b[i-1];
  T t=0;
  for(size_t i=0;i<index.size();i++)
    t+=d[index[i]];
  t/=index.size();
  for(size_t i=0;i<index.size();i++)
    d[index[i]]-=t;
}

#endif /* _BASELINE2DELAYLINELINEAR_H_ */
