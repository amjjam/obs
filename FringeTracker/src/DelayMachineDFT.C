#include "../include/DelayMachineDFT.H"

DelayMachineDFT::DelayMachineDFT(std::string name,
				 int nIncoherent, int nSmooth):
  DelayMachine(name),ipSpecHistory(0),iPeakHistory(0){
  pSpecHistory.resize(nIncoherent);
  peakHistory.resize(nSmooth);
  snrHistory.resize(nSmooth);
}

void DelayMachineDFT::load(const amjInterferometry::Phasors<float> &phasors){
  //
  // Load new power spectrum into history
  //
  
  calcpowerspectrum(phasors,pSpecHistory[ipSpecHistory]);

  //
  // Compute average power spectrum over nIncoherent history
  //
  
  pSpec=pSpecHistory[ipSpecHistory];
  int n=0;
  for(size_t i=0;i<pSpecHistory.size();i++){
    if(i==ipSpecHistory)
      continue;
    if(pSpecHistory[i].size()!=pSpec.size())
      continue;
    pSpec+=pSpecHistory[i];
    n++;
  }
  ipSpecHistory=(ipSpecHistory+1)%pSpecHistory.size();

  pSpec/=n;
  
  //
  // Find the delay and SNR and put it in smooth history
  //
  
  pSpecX=pSpec;
  
  // Find the peak
  unsigned int iMax=maxIndex(pSpecX);
  float max=pSpecX[iMax];
  
  peakHistory[iPeakHistory]=pSpecX(iMax);

  // Zero the 10% largest value
  for(unsigned int i=0;i<pSpecX.size()/10;i++)
    pSpecX[maxIndex(pSpecX)]=0;

  // Find the average of the 90% low values
  int nn=0;
  float sum=0;
  for(unsigned int i=0;i<pSpecX.size();i++)
    if(pSpecX[i]>0){
      sum+=pSpecX[i];
      nn++;
    }
  float avg=sum/nn;
  
  // Find the RMS of the 90% low values around their average
  float tmp;
  sum=0;
  for(unsigned int i=0;i<pSpecX.size();i++)
    if(pSpecX[i]>0){
      tmp=pSpecX[i]-avg;
      sum+=tmp*tmp;
    }  
  snrHistory[iPeakHistory]=(max-avg)/sqrt(sum/nn);

  iPeakHistory=(iPeakHistory+1)%peakHistory.size();

  //
  // Compute smoothed delay and SNR over nSmooth history
  //

  _delay=0;
  _snr=0;
  for(size_t i=0;i<peakHistory.size();i++){
    _delay+=peakHistory[i];
    _snr+=snrHistory[i];
  }
  _delay/=peakHistory.size();
  _snr/=peakHistory.size();
}

// void DelayMachineDFT::computeAveragePSpecPeak(){
//   pSpecX=pSpec;
  
//   // Find the peak
//   unsigned int iMax=findMax(pSpecX.power());
//   float max=pSpecX.power()[iMax];
  
//   peakHistory[iPeakHistory]=pSpecX.delays()[iMax];

//   // Zero the 10% largest value
//   for(unsigned int i=0;i<pSpecX.size()/10;i++)
//     pSpecX.power()[findMax(pSpecX.power())]=0;

//   // Find the average of the 90% low values
//   int nn=0;
//   float sum=0;
//   for(unsigned int i=0;i<pSpecX.size();i++)
//     if(pSpecX.power()[i]>0){
//       sum+=pSpecX.power()[i];
//       nn++;
//     }
//   float avg=sum/nn;

//   // Find the RMS of the 90% low values around their average
//   float tmp;
//   sum=0;
//   for(unsigned int i=0;i<pSpecX.size();i++)
//     if(pSpecX.power()[i]>0){
//       tmp=pSpecX.power()[i]-avg;
//       sum+=tmp*tmp;
//     }  
//   snrHistory[iPeakHistory]=(max-avg)/sqrt(sum/nn);
  
//   iPeakHistory++;
//   if(iPeakHistory==nSmooth) iPeakHistory=0;
// }


// void DelayMachineDFT::computeAveragePSpecPeakSmooth(){
//   _delay={0,0};
//   for(unsigned int i=0;i<nSmooth;i++){
//     _delay.first+=peakHistory[i];
//     _delay.second+=snrHistory[i];
//   }
//   _delay.first/=nSmooth;
//   _delay.second/=nSmooth;
// }



int DelayMachineDFT::maxIndex(const amjInterferometry::PowerSpectrum<float> &p){
  int j=p.size()/2;
  float max=0;
  
  for(size_t i=0;i<p.size();i++)
    if(p[i]>max){
      j=i;
      max=p[i];
    }
  
  return j;
}
