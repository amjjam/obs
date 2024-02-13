#include "../include/FringeTrackerStateMachine.H"


#include <iostream>

#include <amjCom/amjPacket.H>

FringeTrackerStateMachineConfig::
FringeTrackerStateMachineConfig(int onoff,float snrFound,float snrLost,
				int holdSearch,int holdFound,int holdLost,
				float searchAMin,float searchAMax,
				float step,float factor,float maxGD,
				float gain):
  onoff(onoff),snrFound(snrFound),snrLost(snrLost),holdSearch(holdSearch),
  holdFound(holdFound),holdLost(holdLost),searchAMin(searchAMin),
  searchAMax(searchAMax),searchStepSize(step),searchFactor(factor),
  maxGD(maxGD),gain(gain){}

int FringeTrackerStateMachineConfig::size(){
  return sizeof(int)+2*sizeof(float)+3*sizeof(int)+6*sizeof(float);
}

int FringeTrackerStateMachineConfig::write(uint8_t *d){
  uint8_t *dd=(uint8_t *)memcpy(d,&onoff,sizeof(int))+sizeof(int);
  dd=(uint8_t *)memcpy(dd,&snrFound,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&snrLost,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&holdSearch,sizeof(int))+sizeof(int);
  dd=(uint8_t *)memcpy(dd,&holdFound,sizeof(int))+sizeof(int);
  dd=(uint8_t *)memcpy(dd,&holdLost,sizeof(int))+sizeof(int);
  dd=(uint8_t *)memcpy(dd,&searchAMin,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&searchAMax,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&searchStepSize,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&searchFactor,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&maxGD,sizeof(float))+sizeof(float);
  dd=(uint8_t *)memcpy(dd,&gain,sizeof(float))+sizeof(float);
  //std::cout << (int)(dd-d) << std::endl;
  return size();
}

int FringeTrackerStateMachineConfig::read(uint8_t *d){
  uint8_t *dd=d;
  memcpy(&onoff,dd,sizeof(int)); dd+=sizeof(int);
  memcpy(&snrFound,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&snrLost,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&holdSearch,dd,sizeof(int)); dd+=sizeof(int);
  memcpy(&holdFound,dd,sizeof(int)); dd+=sizeof(int);
  memcpy(&holdLost,dd,sizeof(int)); dd+=sizeof(int);
  memcpy(&searchAMin,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&searchAMax,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&searchStepSize,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&searchFactor,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&maxGD,dd,sizeof(float)); dd+=sizeof(float);
  memcpy(&gain,dd,sizeof(float));
  //std::cout << (int)(dd-d) << std::endl;
  //std::cout << size() << std::endl;
  return size();
}

bool FringeTrackerStateMachineConfig::operator==(const FringeTrackerStateMachineConfig &c) const{
  if(onoff!=c.onoff) return false;
  if(rdiff(snrFound,c.snrFound)>0.01) return false;
  if(rdiff(snrLost,c.snrLost)>0.01) return false;
  if(holdSearch!=c.holdSearch) return false;
  if(holdFound!=c.holdFound) return false;
  if(holdLost!=c.holdLost) return false;
  if(rdiff(searchAMin,c.searchAMin)>0.01) return false;
  if(rdiff(searchAMax,c.searchAMax)>0.01) return false;
  if(rdiff(searchStepSize,c.searchStepSize)>0.01) return false;
  if(rdiff(searchFactor,c.searchFactor)>0.01) return false;
  if(rdiff(maxGD,c.maxGD)>0.01) return false;
  if(rdiff(gain,c.gain)>0.01) return false;
  return true;
}
   
void FringeTrackerStateMachineConfig::print(){
  std::cout << "onoff=" << onoff << std::endl;
  std::cout << "snrFound=" << snrFound << std::endl;
  std::cout << "snrLost=" << snrLost << std::endl;
  std::cout << "holdSearch=" << holdSearch << std::endl;
  std::cout << "holdFound=" << holdFound << std::endl;
  std::cout << "holdLost=" << holdLost << std::endl;
  std::cout << "searchAMin=" << searchAMin << std::endl;
  std::cout << "searchAMax=" << searchAMax << std::endl;
  std::cout << "searchStepSize=" << searchStepSize << std::endl;
  std::cout << "searchFactor=" << searchFactor << std::endl;
  std::cout << "maxGD=" << maxGD << std::endl;
  std::cout << "gain=" << gain << std::endl;
}


std::vector<float> FringeTrackerStateMachineStatistics::stats(){
  std::vector<float> _stats(counts.size());
  float sum;
  for(unsigned int i=0;i<counts.size();i++)
    sum+=counts[i];
  for(unsigned int i=0;i<counts.size();i++)
    _stats[i]=(float)counts[i]/sum;
  return _stats;
}

void FringeTrackerStateMachineStatistics::update(int i){
  if(i<0||i>=(int)counts.size()){
    std::cout << "beyond end of array: state=" << i << ", array size="
	      << counts.size() << std::endl;
    exit(EXIT_FAILURE);
  }
  counts[i]++;
}

void FringeTrackerStateMachineStatistics::write(amjPacket &p) const{
  p << counts;
}

void FringeTrackerStateMachineStatistics::read(amjPacket &p){
  p >> counts;
}

std::string FringeTrackerStateMachine::stateName() const{
  return stateName(_state);
}

std::string FringeTrackerStateMachine::stateName(int state) const{
  if(state>STATE_LOST||state<STATE_STOP)
    return "Unknown/Invalid";
  return _stateNames[state];  
}

int FringeTrackerStateMachine::advance(){
  //std::cout << delay << " " << snr << std::endl;
  if(c.onoff==OFF)
    _state=STATE_STOP;
  else{
    if(_state==STATE_STOP)
      _state=STATE_SEARCH;
  }
  if(_state==STATE_STOP){
    _move=0;
      return 0;
  }
  if(_state==STATE_SEARCH){
    // Move delay lines in search pattern
    _move=0;
    if(snr>c.snrFound){
      _state=STATE_FOUND;
      counter=0;
    }
    else{
      counter++;
      if(counter>=c.holdSearch){
	counter=0;
	_move=searchStep;
	cumSearchMove+=searchStep;
	if(fabsf(cumSearchMove)>=searchAmplitude){
	  searchAmplitude*=c.searchFactor;
	  searchStep=-searchStep;
	}
	if(searchAmplitude>c.searchAMax)
	  searchAmplitude=c.searchAMax;
      }
    }
  }
  else if(_state==STATE_FOUND){
    // Don't move delay, just confirm fringe sighting
    _move=0;
    if(snr<c.snrLost){
      _state=STATE_SEARCH;
      counter=0;
      // Should we really reverse search step when going from FOUND to SEARCH?
      // I don't think so. I think we should continue the search where we left off
      // Investigate this
      searchStep=c.searchStepSize;
      // I decide to exclused it for now: c.searchStepSize=(-c.searchStepSize);
    }
    else{
      counter++;
      if(counter>=c.holdFound){
	_state=STATE_LOCK;
      }
    }
  }
  else if(_state==STATE_LOCK){
    // Move delay lines according to tracking information
    if(snr<c.snrLost){
      _state=STATE_LOST;
      counter=0;
      _move=0;
    }
    else{
      _move=delay*c.gain;//(64-iMax[iB])*250;
      if(_move>c.maxGD) _move=c.maxGD;
      if(_move<-c.maxGD) _move=-c.maxGD;
    }
  }
  else if(_state==STATE_LOST){
    // Don't move delay lines, wait for fringes to reappear
    _move=0;
    if(snr>c.snrFound){
      _state=STATE_LOCK;
    }
    else{
      counter++;
      if(counter>=c.holdLost){
	_state=STATE_SEARCH;
	counter=0;
	searchStep=c.searchStepSize;
	cumSearchMove=0;
	searchAmplitude=c.searchAMin;
	c.searchStepSize=-c.searchStepSize;
      }
    }
  }
  else{
    printf("Unknown state: state=%d\n",_state);
    exit(1);
  }

  return 0;
}
