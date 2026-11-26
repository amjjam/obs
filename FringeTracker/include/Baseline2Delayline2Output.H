#ifndef BASELINE2DELAYLINE2OUTPUT_H
#define BASELINE2DELAYLINE2OUTPUT_H

#include "Baseline2DelaylineLinear.H"
#include "../../protocol/include/FringeTrackerProtocol.H"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <utility>

template<typename T=float>
class Baseline2Delayline2Output{
public:
  bool config(const Baseline2DelaylineLinearConfig &);
  bool config(const OutputConfig &);
  // Returns true when dm is ready to send.
  bool operator()(const std::vector<T> &bm,std::vector<T> &dm,int32_t pversion);
private:
  std::mutex m;
  std::unique_ptr<Baseline2Delayline<T>> a;
  int32_t version=0;
  size_t nindex=0;
  size_t lastIndex=0;
  OutputConfig out{std::numeric_limits<double>::infinity(),1,1};
  int32_t istride=0;
};

template<typename T>
bool Baseline2Delayline2Output<T>::config(const Baseline2DelaylineLinearConfig &c){
  std::vector<std::size_t> index;
  index.reserve(c.index.size());
  std::size_t last=0;
  for(auto i:c.index){
    if(!std::in_range<std::size_t>(i))
      return false;
    index.push_back(static_cast<std::size_t>(i));
    last=std::max(last,index.back());
  }
  
  size_t n=index.size();
  std::unique_ptr<Baseline2Delayline<T>> next;
  if(n!=0)
    next=std::make_unique<Baseline2DelaylineLinear<T>>(std::move(index));
  
  std::lock_guard<std::mutex> lock(m);
  a=std::move(next);
  version=c.version;
  nindex=n;
  lastIndex=last;
  istride=0;
  return true;
}

template<typename T>
bool Baseline2Delayline2Output<T>::config(const OutputConfig &c){
  if(!(c.Max>=0)||!std::isfinite(c.Gain))
    return false;
  
  std::lock_guard<std::mutex> lock(m);
  if(c.stride!=out.stride)
    istride=0;
  out=c;
  return true;
}

template<typename T>
bool Baseline2Delayline2Output<T>::operator()(const std::vector<T> &bm,std::vector<T> &dm,int32_t pversion){
    std::lock_guard<std::mutex> lock(m);
    if(!a||version!=pversion||bm.size()<nindex-1||dm.size()<=lastIndex)
      return false;

    if(!(*a)(bm,dm))
      return false;
    
    if(++istride<out.stride)
      return false;
    istride=0;

    for(T &d:dm)
      d=std::clamp(static_cast<double>(d),-out.Max,out.Max)*out.Gain;
    return true;
  }


#endif
