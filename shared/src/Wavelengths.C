#include "../include/Wavelengths.H"

Wavelengths::Wavelengths(std::string name,int nL, float L0, float L1):
  _name(name),_L0(L0),_L1(L1){
  _L.resize(nL);
  float B0=1/L0,B1=1/L1;
  float dB=(B1-B0)/(nL-1);
  for(unsigned int i=0;i<_L.size();i++)
    _L[i]=1/(B0+i*dB);
  
}

Wavelengths::Wavelengths(std::string name, std::vector<float> &L):
  _name(name),_L(L){
  _L0=_L[0];
  _L1=_L[_L.size()-1];
}
