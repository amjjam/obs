#include "../include/DelayMachineGD.H"

DelayMachineGD::DelayMachineGD(std::string name,
			       const Wavelengths &L, int nOver,
			       int nIncoherent, int nSmooth):
  DelayMachine(name,L),nOver(nOver),nIncoherent(nIncoherent),nSmooth(nSmooth),
  ipSpecHistory(0),peakHistory(nSmooth,0),snrHistory(nSmooth,0),
  iPeakHistory(0){
  
  // Compute the size of the FFT to be a power of 2
  unsigned int nFFT=1;
  while(nFFT<nOver*L.size()) nFFT*=2;
  fftData.resize(2*nFFT);
  PowerSpectrumDelays delays(L,nFFT); // Compute delay values
  pSpec=PowerSpectrum(L.name(),delays);
  pSpecHistory.clear();
  for(int i=0;i<nIncoherent;i++)
    pSpecHistory.push_back(PowerSpectrum(L.name(),delays));
}

int DelayMachineGD::load(Phasors &phasors){
  // Add power spectrum to history
  pSpecHistory[ipSpecHistory].fromPhasors(phasors);
  ipSpecHistory=(ipSpecHistory+1)%pSpecHistory.size();

  // Compute the average of the history
  pSpec.zero();
  for(unsigned int i=0;i<nIncoherent;i++)
    pSpec.power()+=pSpecHistory[i].power();
  pSpec.power()/=nIncoherent;

  // Find the peak location and snr
  computeAveragePSpecPeak();
  computeAveragePSpecPeakSmooth();
  
  return 0;
}

std::pair<float,float> DelayMachineGD::delay(){
  return _delay;
}

void DelayMachineGD::computeAveragePSpecPeak(){
  pSpecX=pSpec;
  
  // Find the peak
  unsigned int iMax=findMax(pSpecX.power());
  float max=pSpecX.power()[iMax];
  
  peakHistory[iPeakHistory]=pSpecX.delays()[iMax];

  // Zero the 10% largest value
  for(unsigned int i=0;i<pSpecX.size()/10;i++)
    pSpecX.power()[findMax(pSpecX.power())]=0;

  // Find the average of the 90% low values
  int nn=0;
  float sum=0;
  for(unsigned int i=0;i<pSpecX.size();i++)
    if(pSpecX.power()[i]>0){
      sum+=pSpecX.power()[i];
      nn++;
    }
  float avg=sum/nn;

  // Find the RMS of the 90% low values around their average
  float tmp;
  sum=0;
  for(unsigned int i=0;i<pSpecX.size();i++)
    if(pSpecX.power()[i]>0){
      tmp=pSpecX.power()[i]-avg;
      sum+=tmp*tmp;
    }  
  snrHistory[iPeakHistory]=(max-avg)/sqrt(sum/nn);
  
  iPeakHistory++;
  if(iPeakHistory==nSmooth) iPeakHistory=0;
}


void DelayMachineGD::computeAveragePSpecPeakSmooth(){
  _delay={0,0};
  for(unsigned int i=0;i<nSmooth;i++){
    _delay.first+=peakHistory[i];
    _delay.second+=snrHistory[i];
  }
  _delay.first/=nSmooth;
  _delay.second/=nSmooth;
}



int findMax(PowerSpectrumPower &p){
  int j=p.size()/2;
  float max=0;
  
  for(int i=0;i<p.size();i++)
    if(p[i]>max){
      j=i;
      max=p[i];
    }

  return j;
}
