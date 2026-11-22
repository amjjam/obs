#ifndef _BASELINE2DELAYLINE_H_
#define _BASELINE2DELAYLINE_H_

#include <vector>

template<typename T=float>
class Baseline2Delayline{
public:
  Baseline2Delayline()=default;
  virtual ~Baseline2Delayline()=default;
  virtual void operator()(const std::vector<T> &, std::vector<T> &)=0;
};

#endif /* _BASELINE2DELAYLINE_H_ */
