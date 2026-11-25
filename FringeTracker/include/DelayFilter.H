#ifndef _DELAYFILTER_H_
#define _DELAYFILTER_H_

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <deque>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

template<typename T=float>
class DelayFilter{
public:
  virtual ~DelayFilter()=default;

  virtual void operator()(T delay,T snr)=0;
  T delay() const {return _delay;}
  T snr() const {return _snr;}
protected:
  static std::size_t window_size(double tfilter){
    if(!std::isfinite(tfilter))
      throw std::invalid_argument("Delay filter width must be finite");
    const double width=std::round(std::max(1.0,tfilter));
    if(width>=static_cast<double>(std::numeric_limits<std::size_t>::max()))
      throw std::invalid_argument("Delay filter width is too large");
    return static_cast<std::size_t>(width);
  }

  T _delay{},_snr{};
};

template<typename T=float>
class DelayFilterNone:public DelayFilter<T>{
public:
  void operator()(T delay,T snr) override{
    this->_delay=delay;
    this->_snr=snr;
  }
};

template<typename T=float>
class DelayFilterBoxcar:public DelayFilter<T>{
public:
  explicit DelayFilterBoxcar(double tfilter):n(this->window_size(tfilter)){}

  void operator()(T delay,T snr) override{
    samples.push_back({delay,snr});
    if(samples.size()>n)
      samples.pop_front();

    double sum_delay=0,sum_snr=0;
    for(const auto& sample:samples){
      sum_delay+=sample.delay;
      sum_snr+=sample.snr;
    }
    this->_delay=static_cast<T>(sum_delay/samples.size());
    this->_snr=static_cast<T>(sum_snr/samples.size());
  }
private:
  struct Sample{
    T delay,snr;
  };
  std::size_t n;
  std::deque<Sample> samples;
};

template<typename T=float>
class DelayFilterLeakyBox:public DelayFilter<T>{
public:
  explicit DelayFilterLeakyBox(double tfilter){
    if(!std::isfinite(tfilter))
      throw std::invalid_argument("Leaky-box width must be finite");
    alpha=1.0/std::max(1.0,tfilter);
  }

  void operator()(T delay,T snr) override{
    if(!initialized||alpha==1){
      filtered_delay=delay;
      filtered_snr=snr;
      initialized=true;
    }
    else{
      filtered_delay=alpha*delay+(1-alpha)*filtered_delay;
      filtered_snr=alpha*snr+(1-alpha)*filtered_snr;
    }
    this->_delay=static_cast<T>(filtered_delay);
    this->_snr=static_cast<T>(filtered_snr);
  }
private:
  double alpha;
  bool initialized=false;
  double filtered_delay{},filtered_snr{};
};

template<typename T=float>
class DelayFilterMedian:public DelayFilter<T>{
public:
  explicit DelayFilterMedian(double tfilter):n(this->window_size(tfilter)){}

  void operator()(T delay,T snr) override{
    samples.push_back({delay,snr});
    if(samples.size()>n)
      samples.pop_front();
    this->_delay=median(&Sample::delay);
    this->_snr=median(&Sample::snr);
  }
private:
  struct Sample{
    T delay,snr;
  };

  T median(T Sample::*field){
    values.clear();
    for(const auto& sample:samples){
      const T value=sample.*field;
      if(std::isnan(value))
        return value;
      values.push_back(value);
    }
    const auto middle=values.begin()+values.size()/2;
    std::nth_element(values.begin(),middle,values.end());
    if(values.size()%2)
      return *middle;
    return static_cast<T>(std::midpoint(
      static_cast<long double>(*std::max_element(values.begin(),middle)),
      static_cast<long double>(*middle)));
  }

  std::size_t n;
  std::deque<Sample> samples;
  std::vector<T> values;
};

#endif
