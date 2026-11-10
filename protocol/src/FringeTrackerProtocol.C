#include "../include/FringeTrackerProtocol.H"

#include <cstring>
#include <format>

size_t PhasorsAverageConfig::size(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(uint8_t)+sizeof(float)
    +sizeof(int32_t);
}

size_t PhasorsAverageConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=PHASORSAVERAGECONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(uint32_t);
  std::memcpy(p,&avgType,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&tavg,sizeof(float)); p+=sizeof(float);
  std::memcpy(p,&stride,sizeof(int32_t)); p+=sizeof(uint32_t);
  return p-pp;
}

size_t PhasorsAverageConfig::read(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=PHASORSAVERAGECONFIG)
    throw std::runtime_error
      (std::format("Invalid sync byte for struct PhasorsAverageConfig. "
		   "Expected 0x{:02X}, got 0x{:02X}",PHASORSAVERAGECONFIG,t));
  std::memcpy(&version,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
  std::memcpy(&avgType,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(&tavg,p,sizeof(float)); p+=sizeof(float);
  std::memcpy(&stride,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
  return p-pp;
}

size_t CalcPowerSpectrumConfig::size(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(int16_t)+3*sizeof(float)+sizeof(int16_t);
}

size_t CalcPowerSpectrumConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=CALCPOWERSPECTRUMCONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&calcType,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(p,&dftMin,sizeof(float)); p+=sizeof(float);
  std::memcpy(p,&dftMax,sizeof(float)); p+=sizeof(float);
  std::memcpy(p,&dftStep,sizeof(float)); p+=sizeof(float);
  std::memcpy(p,&nOver,sizeof(int16_t)); p+=sizeof(int16_t);
  return p-pp;
}

size_t CalcPowerSpectrumConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=CALCPOWERSPECTRUMCONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct CalcPowerSpectrumConfig. Expected 0x{:02X}, got 0x{:02X}",CALCPOWERSPECTRUMCONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&calcType,p,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(&dftMin,p,sizeof(float)); p+=sizeof(float);
  std::memcpy(&dftMax,p,sizeof(float)); p+=sizeof(float);
  std::memcpy(&dftStep,p,sizeof(float)); p+=sizeof(float);
  std::memcpy(&nOver,p,sizeof(int16_t)); p+=sizeof(int16_t);
  return p-pp;
}

size_t PowerSpectrumAverageConfig::size(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(int16_t)+sizeof(double);
}

size_t PowerSpectrumAverageConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=POWERSPECTRUMAVERAGECONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&avgType,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(p,&tavg,sizeof(double)); p+=sizeof(double);
  return p-pp;
}

size_t PowerSpectrumAverageConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=POWERSPECTRUMAVERAGECONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct PowerSpectrumAverageConfig. Expected 0x{:02X}, got 0x{:02X}",POWERSPECTRUMAVERAGECONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&avgType,p,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(&tavg,p,sizeof(double)); p+=sizeof(double);
  return p-pp;
}

size_t PowerSpectrumSmoothingConfig::size(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(int16_t)+sizeof(float);
}

size_t PowerSpectrumSmoothingConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=POWERSPECTRUMSMOOTHCONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&smoothType,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(p,&width,sizeof(float)); p+=sizeof(float);
  return p-pp;
}

size_t PowerSpectrumSmoothingConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=POWERSPECTRUMSMOOTHCONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct PowerSpectrumSmoothingConfig. Expected 0x{:02X}, got 0x{:02X}",POWERSPECTRUMSMOOTHCONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&smoothType,p,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(&width,p,sizeof(float)); p+=sizeof(float);
  return p-pp;
}

size_t AnalyzePowerSpectrumConfig::size(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(float);
}

size_t AnalyzePowerSpectrumConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=ANALYZEPOWERSPECTRUMCONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&width,sizeof(float)); p+=sizeof(float);
  return p-pp;
}

size_t AnalyzePowerSpectrumConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=ANALYZEPOWERSPECTRUMCONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct AnalyzePowerSpectrumConfig. Expected 0x{:02X}, got 0x{:02X}",ANALYZEPOWERSPECTRUMCONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&width,p,sizeof(float)); p+=sizeof(float);
  return p-pp;
}

size_t DelayFilterConfig::size(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(int16_t)+sizeof(double);
}

size_t DelayFilterConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=DELAYFILTERCONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&filteringType,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(p,&tfilter,sizeof(double)); p+=sizeof(double);
  return p-pp;
}

size_t DelayFilterConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=DELAYFILTERCONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct DelayFilterConfig. Expected 0x{:02X}, got 0x{:02X}",DELAYFILTERCONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&filteringType,p,sizeof(int16_t)); p+=sizeof(int16_t);
  std::memcpy(&tfilter,p,sizeof(double)); p+=sizeof(double);
  return p-pp;
}

size_t DelayMachineConfig::size(){
  return sizeof(uint8_t)+cp.size()+pa.size()+ps.size()+ap.size()+df.size()+2*sizeof(int32_t);
}

size_t DelayMachineConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=DELAYMACHINECONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  p+=cp.write(p);
  p+=pa.write(p);
  p+=ps.write(p);
  p+=ap.write(p);
  p+=df.write(p);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&stride,sizeof(int32_t)); p+=sizeof(int32_t);
  return p-pp;
}

size_t DelayMachineConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=DELAYMACHINECONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct DelayMachineConfig. Expected 0x{:02X}, got 0x{:02X}",DELAYMACHINECONFIG,t));
  p+=cp.read(p);
  p+=pa.read(p);
  p+=ps.read(p);
  p+=ap.read(p);
  p+=df.read(p);
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&stride,p,sizeof(int32_t)); p+=sizeof(int32_t);
  return p-pp;
}

size_t BaselineNameConfig::size(){
  size_t s=sizeof(uint8_t)+ // sync byte
    sizeof(int32_t)+ // version
    sizeof(uint32_t)+ // size2
    sizeof(int32_t); // number of strings in vector
  for(size_t i=0;i<names.size();i++)
    s+=sizeof(uint32_t)+names[i].size(); /* size of each string */
  return s;
}

size_t BaselineNameConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=BASELINENAMECONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(uint32_t)); p+=sizeof(uint32_t);  
  uint32_t s=sizeof(uint32_t); // number of strings
  for(size_t i=0;i<names.size();i++)
    s+=sizeof(uint32_t)+names[i].size(); // size of each string
  std::memcpy(p,&s,sizeof(uint32_t)); p+=sizeof(uint32_t); // size 2
  s=names.size(); // number of strings
  std::memcpy(p,&s,sizeof(uint32_t)); p+=sizeof(uint32_t);
  for(size_t i=0;i<names.size();i++){
    s=names[i].size(); 
    std::memcpy(p,&s,sizeof(uint32_t)); p+=sizeof(uint32_t); // size of each string
    std::memcpy(p,names[i].data(),s); p+=s; // each string
  }
  return p-pp;
}

size_t BaselineNameConfig::size1(){
  return sizeof(uint8_t)+ // sync byte
    sizeof(int32_t)+ // version
    sizeof(uint32_t); // size2
}

size_t BaselineNameConfig::read1(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t); // sync byte
  if(t!=BASELINENAMECONFIG)
    throw std::runtime_error
      (std::format("Invalid sync byte for struct BaselineNameConfig. "
		   "Expected 0x{:02X}, got 0x{:02X}",BASELINENAMECONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t); // version 
  std::memcpy(&_size2,p,sizeof(uint32_t)); p+=sizeof(uint32_t); // size2
  return p-pp;
}

size_t BaselineNameConfig::size2(){
  return _size2;
}

size_t BaselineNameConfig::read2(uint8_t *pp){
  uint8_t *p=pp;
  uint32_t n,m;
  std::memcpy(&n,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
  names.resize(n);
  for(size_t i=0;i<n;i++){
    std::memcpy(&m,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
    names[i].resize(m);
    std::memcpy(names[i].data(),p,m); p+=m;
  }
  return p-pp;
}

size_t StateMachineConfig::size(){
  return size1()+size2();
}

size_t StateMachineConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=STATEMACHINECONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(int32_t)); p+=sizeof(int32_t);
  uint32_t s=size2();
  std::memcpy(p,&s,sizeof(uint32_t)); p+=sizeof(uint32_t);
  s=bc.size();
  std::memcpy(p,&s,sizeof(uint32_t)); p+=sizeof(uint32_t);
  for(size_t i=0;i<bc.size();i++){
    std::memcpy(p,&bc[i].SNRFound,sizeof(float)); p+=sizeof(float);
    std::memcpy(p,&bc[i].SNRLost,sizeof(float)); p+=sizeof(float);
    std::memcpy(p,&bc[i].TFound,sizeof(int32_t)); p+=sizeof(int32_t);
    std::memcpy(p,&bc[i].TLost,sizeof(int32_t)); p+=sizeof(int32_t);
  }
  std::memcpy(p,&sc.hold,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(p,&sc.Amin,sizeof(double)); p+=sizeof(double);
  std::memcpy(p,&sc.Amax,sizeof(double)); p+=sizeof(double);
  std::memcpy(p,&sc.step,sizeof(double)); p+=sizeof(double);
  std::memcpy(p,&sc.factor,sizeof(double)); p+=sizeof(double);
  std::memcpy(p,&stride,sizeof(int32_t)); p+=sizeof(int32_t);
  return p-pp;
}

size_t StateMachineConfig::size1(){
  return sizeof(uint8_t)+sizeof(int32_t)+sizeof(uint32_t);
}

size_t StateMachineConfig::read1(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=STATEMACHINECONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct StateMachineConfig. Expected 0x{:02X}, got 0x{:02X}",STATEMACHINECONFIG,t));
  std::memcpy(&version,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&_size2,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
  return p-pp;
}

size_t StateMachineConfig::size2(){
  return sizeof(uint32_t)+bc.size()*(2*sizeof(float)+2*sizeof(int32_t))+sizeof(int32_t)+4*sizeof(double)+sizeof(int32_t);
}

size_t StateMachineConfig::read2(uint8_t *pp){
  uint8_t *p=pp;
  uint32_t n;
  std::memcpy(&n,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
  bc.resize(n);
  for(size_t i=0;i<bc.size();i++){
    std::memcpy(&bc[i].SNRFound,p,sizeof(float)); p+=sizeof(float);
    std::memcpy(&bc[i].SNRLost,p,sizeof(float)); p+=sizeof(float);
    std::memcpy(&bc[i].TFound,p,sizeof(int32_t)); p+=sizeof(int32_t);
    std::memcpy(&bc[i].TLost,p,sizeof(int32_t)); p+=sizeof(int32_t);
  }
  std::memcpy(&sc.hold,p,sizeof(int32_t)); p+=sizeof(int32_t);
  std::memcpy(&sc.Amin,p,sizeof(double)); p+=sizeof(double);
  std::memcpy(&sc.Amax,p,sizeof(double)); p+=sizeof(double);
  std::memcpy(&sc.step,p,sizeof(double)); p+=sizeof(double);
  std::memcpy(&sc.factor,p,sizeof(double)); p+=sizeof(double);
  std::memcpy(&stride,p,sizeof(int32_t)); p+=sizeof(int32_t);
  return p-pp;
}

size_t OutputConfig::size(){
  return sizeof(uint8_t)+2*sizeof(double)+sizeof(int32_t);
}

size_t OutputConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=OUTPUTCONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&Max,sizeof(double)); p+=sizeof(double);
  std::memcpy(p,&Gain,sizeof(double)); p+=sizeof(double);
  std::memcpy(p,&stride,sizeof(int32_t)); p+=sizeof(int32_t);
  return p-pp;
}

size_t OutputConfig::read(uint8_t *pp){
  uint8_t *p=pp,t;
  std::memcpy(&t,p,sizeof(uint8_t)); p+=sizeof(uint8_t);
  if(t!=OUTPUTCONFIG)
    throw std::runtime_error(std::format("Invalid sync byte for struct OutputConfig. Expected 0x{:02X}, got 0x{:02X}",OUTPUTCONFIG,t));
  std::memcpy(&Max,p,sizeof(double)); p+=sizeof(double);
  std::memcpy(&Gain,p,sizeof(double)); p+=sizeof(double);
  std::memcpy(&stride,p,sizeof(int32_t)); p+=sizeof(int32_t);
  return p-pp;
}
