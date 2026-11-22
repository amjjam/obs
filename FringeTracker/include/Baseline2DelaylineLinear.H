#ifndef _BASELINE2DELAYLINELINEAR_H_
#define _BASELINE2DELAYLINELINEAR_H_

#include "Baseline2Delayline.H"
#include <numeric>

// Combines baseline delays to make delayline delays. The delays are
// combined in this order:
// 1) set D[index[0]]=0,
// 2) make D[index[i]]=D[index[i-1]]+sign[index[i-1]]*B[i-1]
// 3) Compute the average of D[index] and subtract it from D[index] to make the average 0
// Values of D not touched by index are left unchanged
// default values for sign are 1
template<typename T=float>
class Baseline2DelaylineLinear: public Baseline2Delayline<T>{
public:
  Baseline2DelaylineLinear(std::vector<size_t> index,
			   std::vector<float> sign={});
  virtual ~Baseline2DelaylineLinear()=default;
  void operator()(const std::vector<T> &, std::vector<T> &) override;
private:
  std::vector<size_t> index;
  std::vector<float> sign;
};

template<typename T>
Baseline2DelaylineLinear<T>::Baseline2DelaylineLinear(std::vector<size_t> index,
						      std::vector<float> sign)
  :index(index){
  if(sign.size()<index.size())
    sign.assign(index.size(),1);
}

template<typename T>
void Baseline2DelaylineLinear<T>::operator()(const std::vector<T> &b,
					     std::vector<T> &d){
  d[index[0]]=0;
  for(size_t i=1;i<index.size();i++)
    d[index[i]]=d[index[i-1]]+b[i-1]*sign[i-1];
  T t=0;
  for(size_t i=0;i<index.size();i++)
    t+=d[index[i]];
  t/=index.size();
  for(size_t i=0;i<index.size();i++)
    d[index[i]]-=t;
}

#endif /* _BASELINE2DELAYLINELINEAR_H_ */
