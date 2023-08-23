#include "../include/Baseline2DelaylineLinear.H"

Baseline2DelaylineLinear::Baseline2DelaylineLinear(int nB, int nD,
						   std::vector<int> i):
  Baseline2Delayline(nB,nD),index(i){
  if(nD<nB+1)
    nD=nB+1;
  while((int)index.size()<nD)
    index.push_back(index.size());
}

Delays<float> Baseline2DelaylineLinear::delaylineMovements(){
  double sum=0;
  for(unsigned int i=0;i<D.size();i++)
    D[index[0]]=0;
  for(unsigned int i=0;i<B.size();i++){
    D[index[i+1]]=D[index[i]]+B[i];
    //sum+=D[index[i+1]];
    //sum+=B[i];
  }
  for(unsigned int i=0;i<D.size();i++)
    sum+=D[i];
  double avg=sum/index.size();
  for(unsigned int i=0;i<index.size();i++)
    D[index[i]]-=avg;
  
  return D;
}
