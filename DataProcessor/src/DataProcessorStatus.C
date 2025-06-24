#include "../include/DataProcessorStatus.H"

#include <cstring>

int DataProcessorStatus::size(){
  return sizeof(char)+sizeof(bool)+sizeof(uint32_t)+biastime.size()
    +sizeof(float);
}

void DataProcessorStatus::write(uint8_t *d){
  memcpy(d,&state,sizeof(char));
  memcpy(d+sizeof(char),&hasBias,sizeof(bool));
  memcpy(d+sizeof(char)+sizeof(bool),&nBias,sizeof(uint32_t));
  biastime.write(d+sizeof(char)+sizeof(bool)+sizeof(uint32_t));
  memcpy(d+sizeof(char)+sizeof(bool)+sizeof(uint32_t)+biastime.size(),
	 &fps,sizeof(float));
}

void DataProcessorStatus::read(uint8_t *d){
  memcpy(&state,d,sizeof(char));
  memcpy(&hasBias,d+sizeof(char),sizeof(bool));
  memcpy(&nBias,d+sizeof(char)+sizeof(bool),sizeof(uint32_t));
  biastime.read(d+sizeof(char)+sizeof(bool)+sizeof(uint32_t));
  memcpy(&fps,d+sizeof(char)+sizeof(bool)+sizeof(uint32_t)+biastime.size(),
	 sizeof(float));
}
