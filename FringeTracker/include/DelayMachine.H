#ifndef _DELAYMACHINE_H_
#define _DELAYMAHCINE_H_

#include <memory>

#include <amjInterferometry.H>

template<typename T=float>
class DelayMachine{
public:
  DelayMachine();
  ~DelayMachine();
  void set_calculator(std::unique_ptr<amjInterferometry::CalcPowerSpectrum<T>>);
  amjInterferometry::CalcPowerSpectrum<T> &calculator();
  void set_averager(std::unique_ptr<amjInterferometry::PowerSpectrumAverage<T>>);
  amjInterferometry::PowerSpectrumAverage<T> &averager();
  void set_smoother(std::function<void(const amjInterferometry::PowerSpectrum<T> &,T)>);
  void set_smoother_width(T);
  void set_analyzer(std::unique_ptr<amjInterferometry::AnalyzePowerSpectrum<T>>);
  amjInterferometry::AnalyzePowerSpectrum<T> &analyzer();
  //void set_filter(std::unique_ptr<DelayFilter<T>>);
  //DelayFilter<T> &filter();
  void load(const amjInterferometry::Phasors<T> &);
  const amjInterferometry::PowerSpectrum<T> &p(){return _pa;}
  T delay(){return _delay;}
  T snr(){return _snr;}
private:
  std::unique_ptr<amjInterferometry::CalcPowerSpectrum<T>> _cp;
  std::unique_ptr<amjInterferometry::PowerSpectrumAverage<T>> _pa;
  std::function<void(const amjInterferometry::PowerSpectrum<T> &,T)> _smoother;
  T _smoother_width;
  std::unique_ptr<amjInterferometry::AnalyzePowerSpectrum<T>> _ap;
  //std::unique_ptr<DelayFilter<T>> _df;
  T _delay,_snr;
};

template<typename T>
amjInterferometry::CalcPowerSpectrum<T> &DelayMachine<T>::calculator(){
  if(!_cp) throw std::runtime_error("No calculator installed");
  return *_cp;
}

template<typename T>
amjInterferometry::PowerSpectrumAverage<T> &DelayMachine<T>::averager(){
  if(!_pa) throw std::runtime_error("No averager installed");
  return *_pa;
}

template<typename T>
void DelayMachine<T>::set_smoother(std::function<void(const amjInterferometry::PowerSpectrum<T> &,T)> _s){
  _smoother=_s;
}

template<typename T>
void DelayMachine<T>::set_smoother_width(T w){
  _smoother_width=w;
}

template<typename T>
amjInterferometry::AnalyzePowerSpectrum<T> &DelayMachine<T>::analyzer(){
  if(!_ap) throw std::runtime_error("No analyzer installed");
  return *_ap;
}

template<typename T>
DelayFilter<T> &DelayMachine<T>::filter(){
  if(!_df) throw std::runtime_error("No filter installed");
  return *_df;
}

template<typename T>
void DelayMachine<T>::load(const amjInterferometry::Phasors<T> &p){
  amjInterferometry::PowerSpectrum<T> ps;
  (*_cp)(p,ps);
  (*_pa).add(ps);
  if(_smoother)
    (*_ap)(_smooth(*_pa,_smoother_width),_delay,_snr);
  else
    (*_ap)(*_pa,_delay,_snr);
  /* here will go the filtering of delay values */
}

#endif
