#ifndef _MACHINES_H_
#define _MACHINES_H_

#include <memory>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include <amjInterferometry.H>
namespace ai=amjInterferometry;
#include "../../protocol/include/FringeTrackerProtocol.H"

//
// PhasorsAverage
//

template<typename T=float>
class PhasorsAverage{
public:
  void config(const std::vector<ai::Phasors<T>>& p);
  void config(const PhasorsAverageConfig& next);
  const PhasorsAverageConfig& config() const {return c;}
  std::size_t size() const {return a.size();}
  
  ai::PhasorsAverage<T>& operator[](std::size_t i){return *a[i];}
  const ai::PhasorsAverage<T>& operator[](std::size_t i) const {return *a[i];}
  bool stride();
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {return std::lock_guard<std::mutex>(m);}
private:
  static std::unique_ptr<ai::PhasorsAverage<T>> make
  (const ai::Phasors<T>& p,const PhasorsAverageConfig& cfg);

  PhasorsAverageConfig c{0,PhasorsAverageConfig::AverageType::Boxcar,1,1};
  std::vector<std::unique_ptr<ai::PhasorsAverage<T>>> a;
  int32_t istride=0;
  mutable std::mutex m;
};

template<typename T>
void PhasorsAverage<T>::config(const std::vector<ai::Phasors<T>>& p){
  std::lock_guard<std::mutex> tmp=lock();
  a.resize(p.size());
  for(size_t i=0;i<p.size();i++)
    a[i]=make(p[i],c);
}

template<typename T>
void PhasorsAverage<T>::config(const PhasorsAverageConfig& next){
  std::lock_guard<std::mutex> tmp=lock();
  if(next.avgType!=c.avgType){
    for(auto& average:a)
      average=make(*average,next);
  }
  else if(next.tavg!=c.tavg){
    for(auto& average:a)
      average->tavg(next.tavg);
  }  
  c=next;
}

template<typename T>
std::unique_ptr<ai::PhasorsAverage<T>> PhasorsAverage<T>::make
(const ai::Phasors<T>& p,const PhasorsAverageConfig& cfg){
  switch(cfg.avgType){
  case PhasorsAverageConfig::AverageType::Boxcar:
    return std::make_unique<ai::PhasorsBoxcar<T>>(p,cfg.tavg);
  case PhasorsAverageConfig::AverageType::LeakyBox:
    return std::make_unique<ai::PhasorsLeakyBox<T>>(p,cfg.tavg);
  }
  throw std::invalid_argument("Invalid phasors average type");
}

template<typename T>
bool PhasorsAverage<T>::stride(){
  if(++istride<c.stride)
    return false;
  istride=0;
  return true;
}

//
// CalcPowerSpectrum
//

template<typename T=float>
class CalcPowerSpectrum{
public:
  void config(const std::vector<ai::Phasors<T>>& p);
  void config(const CalcPowerSpectrumConfig& next);
  const CalcPowerSpectrumConfig& config() const {return c;}
  std::size_t size() const {return a.size();}

  ai::CalcPowerSpectrum<T>& operator[](size_t i){return *a[i];}
  const ai::CalcPowerSpectrum<T>& operator[](size_t i) const {return *a[i];}
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {
    return std::lock_guard<std::mutex>(m);}

private:
  static std::unique_ptr<ai::CalcPowerSpectrum<T>> make
  (const ai::Phasors<T> &,const CalcPowerSpectrumConfig &);

  CalcPowerSpectrumConfig c{0,CalcPowerSpectrumConfig::CalcType::FFT,0,0,0,1};
  std::vector<ai::Phasors<T>> p;
  std::vector<std::unique_ptr<ai::CalcPowerSpectrum<T>>> a;
  mutable std::mutex m;
};

template<typename T>
void CalcPowerSpectrum<T>::config(const std::vector<ai::Phasors<T>>& nextp){
  std::lock_guard<std::mutex> tmp=lock();
  p=nextp;
  a.resize(p.size());
  for(size_t i=0;i<p.size();i++)
    a[i]=make(p[i],c);
}

template<typename T>
void CalcPowerSpectrum<T>::config(const CalcPowerSpectrumConfig& nextc){
  std::lock_guard<std::mutex> tmp=lock();
  c=nextc;
  for(size_t i=0;i<a.size();i++)
    a[i]=make(p[i],c);
}

template<typename T>
std::unique_ptr<ai::CalcPowerSpectrum<T>> CalcPowerSpectrum<T>::make
(const ai::Phasors<T>& p,const CalcPowerSpectrumConfig& c){
  switch(c.calcType){
  case CalcPowerSpectrumConfig::CalcType::FFT:
    return std::make_unique<ai::CalcPowerSpectrumFFT<T>>(p,c.nOver);
  case CalcPowerSpectrumConfig::CalcType::DFT:{
    std::vector<T> delay(floor((c.dftMax-c.dftMin)/c.dftStep)+1);
    for(size_t i=0;i<delay.size();i++)
      delay[i]=static_cast<T>(c.dftMin+i*c.dftStep);
    return std::make_unique<ai::CalcPowerSpectrumDFT<T>>(delay);
  }
  default:
    throw std::invalid_argument("Invalid power spectrum calculator type");
  }
}

//
// PowerSpectrumAverage
//

template<typename T=float>
class PowerSpectrumAverage{
public:
  void config(std::size_t n);
  void config(const PowerSpectrumAverageConfig& next);
  const PowerSpectrumAverageConfig& config() const {return c;}
  std::size_t size() const {return a.size();}

  ai::PowerSpectrumAverage<T>& operator[](std::size_t i){return *a[i];}
  const ai::PowerSpectrumAverage<T>& operator[](std::size_t i) const {return *a[i];}
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {
    return std::lock_guard<std::mutex>(m);}
  
private:
  static std::unique_ptr<ai::PowerSpectrumAverage<T>> make
  (const PowerSpectrumAverageConfig& cfg);

  PowerSpectrumAverageConfig c
    {0,PowerSpectrumAverageConfig::AverageType::Boxcar,1};
  std::vector<std::unique_ptr<ai::PowerSpectrumAverage<T>>> a;
  mutable std::mutex m;
};

template<typename T>
void PowerSpectrumAverage<T>::config(std::size_t n){
  std::lock_guard<std::mutex> tmp=lock();
  a.resize(n);
  for(auto &ia:a)
    ia=make(c);
}

template<typename T>
void PowerSpectrumAverage<T>::config(const PowerSpectrumAverageConfig& next){
  std::lock_guard<std::mutex> tmp=lock();
  if(next.avgType!=c.avgType||a.empty())
    for(auto &ia:a)
      ia=make(next);
  else if(next.tavg!=c.tavg){
    for(auto& ia:a)
      ia->tavg(next.tavg);
  }
  c=next;
}

template<typename T>
std::unique_ptr<ai::PowerSpectrumAverage<T>> PowerSpectrumAverage<T>::make
(const PowerSpectrumAverageConfig& cfg){
  switch(cfg.avgType){
  case PowerSpectrumAverageConfig::AverageType::Boxcar:
    return std::make_unique<ai::PowerSpectrumBoxcar<T>>(cfg.tavg);
  case PowerSpectrumAverageConfig::AverageType::LeakyBox:
    return std::make_unique<ai::PowerSpectrumLeakyBox<T>>(cfg.tavg);
  }
  throw std::invalid_argument("Invalid power spectrum average type");
}


//
// PowerSpectrumSmooth
//

template<typename T=float>
class PowerSpectrumSmooth{
public:
  void config(std::size_t n);
  void config(const PowerSpectrumSmoothingConfig& next);
  const PowerSpectrumSmoothingConfig& config() const {return c;}
  std::size_t size() const {return a.size();}
  
  ai::PowerSpectrumSmooth<T>& operator[](std::size_t i){return *a[i];}
  const ai::PowerSpectrumSmooth<T>& operator[](std::size_t i) const {return *a[i];}
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {
    return std::lock_guard<std::mutex>(m);}
private:
  static std::unique_ptr<ai::PowerSpectrumSmooth<T>> make
  (const PowerSpectrumSmoothingConfig& cfg);
  PowerSpectrumSmoothingConfig c{0,PowerSpectrumSmoothingConfig::SmoothingType::None,0};
  std::vector<std::unique_ptr<ai::PowerSpectrumSmooth<T>>> a;
  mutable std::mutex m;
};

template<typename T>
void PowerSpectrumSmooth<T>::config(std::size_t n){
  std::lock_guard<std::mutex> tmp=lock();
  a.resize(n);
  for(auto& ia:a)
    ia=make(c);
}

template<typename T>
void PowerSpectrumSmooth<T>::config(const PowerSpectrumSmoothingConfig& next){
  std::lock_guard<std::mutex> tmp=lock();
  if(next.smoothType!=c.smoothType)
    for(auto& ia:a)
      ia=make(next);
  else if(next.width!=c.width){
    for(auto& ia:a)
      ia->width(next.width);
  }
  c=next;
}

template<typename T>
std::unique_ptr<ai::PowerSpectrumSmooth<T>> PowerSpectrumSmooth<T>::make
(const PowerSpectrumSmoothingConfig& cfg){
  switch(cfg.smoothType){
  case PowerSpectrumSmoothingConfig::SmoothingType::None:
    return std::make_unique<ai::PowerSpectrumSmoothNone<T>>(cfg.width);
  case PowerSpectrumSmoothingConfig::SmoothingType::Boxcar:
    return std::make_unique<ai::PowerSpectrumSmoothBoxcar<T>>(cfg.width);
  case PowerSpectrumSmoothingConfig::SmoothingType::Gaussian:
    return std::make_unique<ai::PowerSpectrumSmoothGaussian<T>>(cfg.width);
  }
  throw std::invalid_argument("Invalid power spectrum smoothing type");
}

//
// AnalyzePowerSpectrum
//

template<typename T=float>
class AnalyzePowerSpectrum{
public:
  void config(std::size_t n);
  void config(const AnalyzePowerSpectrumConfig& next);
  const AnalyzePowerSpectrumConfig& config() const {return c;}
  std::size_t size() const {return a.size();}

  ai::AnalyzePowerSpectrum<T>& operator[](std::size_t i){return a[i];}
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {
    return std::lock_guard<std::mutex>(m);}

private:
  AnalyzePowerSpectrumConfig c{0,40};
  std::vector<ai::AnalyzePowerSpectrum<T>> a;
  mutable std::mutex m;
};

template<typename T>
void AnalyzePowerSpectrum<T>::config(std::size_t n){
  std::lock_guard<std::mutex> tmp=lock();
  a=std::vector<ai::AnalyzePowerSpectrum<T>>(n);
  for(auto& analyzer:a)
    analyzer.set_peak_width(c.width);
}

template<typename T>
void AnalyzePowerSpectrum<T>::config(const AnalyzePowerSpectrumConfig& next){
  std::lock_guard<std::mutex> tmp=lock();
  if(next.width!=c.width)
    for(auto& analyzer:a)
      analyzer.set_peak_width(next.width);
  c=next;
}

//
// DelayFilters
//

#include "DelayFilter.H"

template<typename T=float>
class DelayFilters{
public:
  void config(std::size_t n);
  void config(const DelayFilterConfig& next);
  const DelayFilterConfig& config() const {return c;}
  std::size_t size() const {return a.size();}

  DelayFilter<T>& operator[](std::size_t i){return *a[i];}
  const DelayFilter<T>& operator[](std::size_t i) const {return *a[i];}
  bool stride();
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {
    return std::lock_guard<std::mutex>(m);}

private:
  static std::unique_ptr<DelayFilter<T>> make(const DelayFilterConfig& cfg);

  DelayFilterConfig c{0,DelayFilterConfig::FilterType::None,0,1};
  std::vector<std::unique_ptr<DelayFilter<T>>> a;
  int32_t istride=0;
  mutable std::mutex m;
};

template<typename T>
void DelayFilters<T>::config(std::size_t n){
  auto guard=lock();
  a.resize(n);
  for(auto& filter:a)
    filter=make(c);
}

template<typename T>
void DelayFilters<T>::config(const DelayFilterConfig& next){
  auto guard=lock();
  if(next.filteringType!=c.filteringType||next.tfilter!=c.tfilter){
    for(auto& filter:a)
      filter=make(next);
  }
  c=next;
}

template<typename T>
bool DelayFilters<T>::stride(){
  if(++istride<c.stride)
    return false;
  istride=0;
  return true;
}

template<typename T>
std::unique_ptr<DelayFilter<T>> DelayFilters<T>::make(const DelayFilterConfig& cfg){
  switch(cfg.filteringType){
  case DelayFilterConfig::FilterType::None:
    return std::make_unique<DelayFilterNone<T>>();
  case DelayFilterConfig::FilterType::Boxcar:
    return std::make_unique<DelayFilterBoxcar<T>>(cfg.tfilter);
  case DelayFilterConfig::FilterType::LeakyBox:
    return std::make_unique<DelayFilterLeakyBox<T>>(cfg.tfilter);
  case DelayFilterConfig::FilterType::Median:
    return std::make_unique<DelayFilterMedian<T>>(cfg.tfilter);
  }
  throw std::invalid_argument("Unsupported delay filter type");
}


//
// StateMachines
//

#include "StateMachine.H"
namespace ai=amjInterferometry;

class StateMachines{
public:
  StateMachines(){c.stride=1;}
  void config(const std::vector<ai::Phasors<>>& ph,int state=STATE_STOP);
  bool config(const StateMachinesConfig& next);
  const StateMachinesConfig& config() const {return c;}
  std::size_t size() const {return a.size();}

  StateMachine& operator[](std::size_t i){return a[i];}
  const StateMachine& operator[](std::size_t i) const {return a[i];}
  bool stride();
  [[nodiscard]] std::lock_guard<std::mutex> lock() const {return std::lock_guard<std::mutex>(m);}
private:
  StateMachinesConfig c{};
  std::vector<StateMachine> a;
  int32_t istride=0;
  mutable std::mutex m;
};

inline void StateMachines::config(const std::vector<ai::Phasors<>>& ph,int state){
  auto guard=lock();
  StateMachineConfig defaults;
  a.clear();
  a.reserve(ph.size());
  for(const auto& iph:ph)
    a.emplace_back(iph.name(),state);
  c.bc.assign(ph.size(),{defaults.snrFound,defaults.snrLost,
			 defaults.holdFound,defaults.holdLost});
  c.sc={defaults.holdSearch,defaults.searchAMin,defaults.searchAMax,
        defaults.searchStepSize,defaults.searchFactor};
  c.version=0;
  c.stride=1;
  istride=0;
}

inline bool StateMachines::config(const StateMachinesConfig& next){
  auto guard=lock();
  if(next.bc.size()!=a.size())
    return false;

  c.bc=next.bc;
  c.sc=next.sc;
  c.version=next.version;
  c.stride=next.stride;
  for(std::size_t i=0;i<a.size();i++){
    auto cfg=a[i].getConfig();
    cfg.snrFound=c.bc[i].SNRFound;
    cfg.snrLost=c.bc[i].SNRLost;
    cfg.holdFound=c.bc[i].TFound;
    cfg.holdLost=c.bc[i].TLost;
    cfg.holdSearch=c.sc.hold;
    cfg.searchAMin=static_cast<float>(c.sc.Amin);
    cfg.searchAMax=static_cast<float>(c.sc.Amax);
    cfg.searchStepSize=static_cast<float>(c.sc.step);
    cfg.searchFactor=static_cast<float>(c.sc.factor);
    a[i].setConfig(cfg);
  }
  return true;
}

inline bool StateMachines::stride(){
  if(++istride<c.stride)
    return false;
  istride=0;
  return true;
}



#endif


