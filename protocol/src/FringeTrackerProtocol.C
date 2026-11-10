#include "../inclucde/FringeTrackerProtocol.H"

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

size_t BaselineNameConfig::size(){
  size_t s=sizeof(uint8_t)+ // sync byte
    sizeof(int32_t)+ // version
    sizeof(uint32_t)+ // size2
    sizeof(int32_t); // number of strings in vector
  for(size_t i=0;i<name.size();i++)
    s+=sizeof(uint32_t)+name[i].size(); /* size of each string */
  return s;
}

size_t BaselineNameConfig::write(uint8_t *pp){
  uint8_t *p=pp;
  uint8_t t=BASELINENAMECONFIG;
  std::memcpy(p,&t,sizeof(uint8_t)); p+=sizeof(uint8_t);
  std::memcpy(p,&version,sizeof(uint32_t)); p+sizeof(uint32_t);  
  uint32_t s=sizeof(uint32_t); // number of strings
  for(size_t i=0;i<name.size();i++)
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
  for(size_t i=0;i<n;i++){
    std::memcpy(&m,p,sizeof(uint32_t)); p+=sizeof(uint32_t);
    names[i].resize(m);
    std::memcpy(names[i].data(),p,m); p+=m;
  }
  return p-pp;
}
