#ifndef _DELAYMACHINEGD_H_
#define _DELAYMACHINEGD_H_

#include "DelayMachine.H"
#include "../../shared/include/PowerSpectrum.H"

#include <mutex>

class DelayMachineGD: public DelayMachine{
public:
  DelayMachineGD(){};
  DelayMachineGD(std::string name, const Wavelengths &L, int nOver,
		 int nIncoherent, int nSmooth);
  virtual ~DelayMachineGD(){};
  virtual int load(Phasors &p);
  virtual std::pair<float,float> delay();
  PowerSpectrum &p(){return pSpec;};
private:
  unsigned int nOver;
  unsigned int nIncoherent;
  unsigned int nSmooth;

  std::vector<float> fftData;
  PowerSpectrum pSpec;
  std::vector<PowerSpectrum> pSpecHistory;
  unsigned int ipSpecHistory;

  PowerSpectrum pSpecX;
  std::vector<float> peakHistory;
  std::vector<float> snrHistory;
  unsigned int iPeakHistory;
  
  float scale;

  std::pair<float,float> _delay;
  void computePSpec();
  void computeAveragePSpec();
  void computeAveragePSpecPeak();
  void computeAveragePSpecPeakSmooth();
};

int findMax(PowerSpectrumPower &p);

#endif /* _DELAYMACHINEGD_H_ */
