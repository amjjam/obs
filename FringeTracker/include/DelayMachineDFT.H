#ifndef _DELAYMACHINEDFT_H_
#define _DELAYMACHINEDFT_H_

#include "DelayMachine.H"
#include <amjInterferometry.H>

#include <mutex>

class DelayMachineDFT: public DelayMachine{
public:
  DelayMachineDFT(){};
  DelayMachineDFT(std::string name, int, int);
  virtual ~DelayMachineDFT(){};
  virtual void load(const amjInterferometry::Phasors<float> &p);
  virtual float delay() const {return _delay;};
  virtual float snr() const {return _snr;}
  const amjInterferometry::PowerSpectrum<float> &p() const {return pSpec;};
private:
  amjInterferometry::CalcPowerSpectrumDFT<float> calcpowerspectrum;
  amjInterferometry::PowerSpectrum<float> pSpec;
  std::vector<amjInterferometry::PowerSpectrum<float> > pSpecHistory;
  unsigned int ipSpecHistory;

  amjInterferometry::PowerSpectrum<float> pSpecX;
  std::vector<float> peakHistory;
  std::vector<float> snrHistory;
  unsigned int iPeakHistory;
  
  float _delay;
  float _snr;
  
  int maxIndex(const amjInterferometry::PowerSpectrum<float> &);
};


#endif /* _DELAYMACHINEDFT_H_ */
