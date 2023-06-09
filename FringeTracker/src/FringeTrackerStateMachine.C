#include "../include/FringeTrackerStateMachine.H"

#include <iostream>

FringeTrackerStateMachineConfig::
FringeTrackerStateMachineConfig(int state,float foundSNR,float lostSNR,
				int holdSearch,int holdFound,int holdLost,
				float searchAMin,float searchAMax,
				float step,float factor,float maxGD,
				float gain):
  _state(state),_snrFound(foundSNR),_snrLost(lostSNR),_holdSearch(holdSearch),
  _holdFound(holdFound),_holdLost(holdLost),_searchAMin(searchAMin),
  _searchAMax(searchAMax),_searchStepSize(step),_searchFactor(factor),
  _maxGD(maxGD),_gain(gain){}

std::string FringeTrackerStateMachine::stateName() const{
  return stateName(_state);
}

std::string FringeTrackerStateMachine::stateName(int state) const{
  if(state>STATE_LOST||state<STATE_STOP)
    return "Unknown/Invalid";
  return _stateNames[state];  
}

int FringeTrackerStateMachine::advance(){
  std::cout << delay << " " << snr << std::endl;
  if(_state==STATE_STOP){
    _move=0;
      return 0;
  }
  if(_state==STATE_SEARCH){
    // Move delay lines in search pattern
    _move=0;
    if(snr>c.snrFound()){
      _state=STATE_FOUND;
      counter=0;
    }
    else{
      counter++;
      if(counter>=c.holdSearch()){
	counter=0;
	_move=searchStep;
	cumSearchMove+=searchStep;
	if(fabsf(cumSearchMove)>=searchAmplitude){
	  searchAmplitude*=c.searchFactor();
	  searchStep=-searchStep;
	}
	if(searchAmplitude>c.searchAMax())
	  searchAmplitude=c.searchAMax();
      }
    }
  }
  else if(_state==STATE_FOUND){
    // Don't move delay, just confirm fringe sighting
    _move=0;
    if(snr<c.snrLost()){
      _state=STATE_SEARCH;
      counter=0;
      // Should we really reverse search step when going from FOUND to SEARCH?
      // I don't think so. I think we should continue the search where we left off
      // Investigate this
      searchStep=c.searchStepSize();
      c.searchStepSize(-c.searchStepSize());
    }
    else{
      counter++;
      if(counter>=c.holdFound()){
	_state=STATE_LOCK;
      }
    }
  }
  else if(_state==STATE_LOCK){
    // Move delay lines according to tracking information
    if(snr<c.snrLost()){
      _state=STATE_LOST;
      counter=0;
      _move=0;
    }
    else{
      _move=delay*c.gain();//(64-iMax[iB])*250;
      if(_move>c.maxGD()) _move=c.maxGD();
      if(_move<-c.maxGD()) _move=-c.maxGD();
    }
  }
  else if(_state==STATE_LOST){
    // Don't move delay lines, wait for fringes to reappear
    _move=0;
    if(snr>c.snrFound()){
      _state=STATE_LOCK;
    }
    else{
      counter++;
      if(counter>=c.holdLost()){
	_state=STATE_SEARCH;
	counter=0;
	searchStep=c.searchStepSize();
	cumSearchMove=0;
	searchAmplitude=c.searchAMin();
	c.searchStepSize(-c.searchStepSize());
      }
    }
  }
  else{
    printf("Unknown state: state=%d\n",_state);
    exit(1);
  }

  return 0;
}
