#ifndef _FRINGETRACKERSTATEMACHINE_H_
#define _FRINGETRACKERSTATEMACHINE_H_

#include <string>
#include <vector>
#include <complex>

#include <amjPacketRW.H>

#define STATE_STOP 0
#define STATE_SEARCH 1
#define STATE_FOUND 2
#define STATE_LOCK 3
#define STATE_LOST 4

class FringeTrackerStateMachineConfig{
public:
  FringeTrackerStateMachineConfig(int state=STATE_STOP, float foundSNR=6,
				  float lostSNR=3, int holdSearch=10,
				  int holdFound=3, int holdLost=10,
				  float searchAMin=3,
				  float searchAMax=5000,
				  float step=3, float factor=1.5,
				  float maxGD=2, float gain=0.01);
  ~FringeTrackerStateMachineConfig(){};
  int state() const {return _state;};
  void state(int state_){_state=state_;};
  float snrFound() const {return _snrFound;};
  void snrFound(float snrFound_) {_snrFound=snrFound_;};
  float snrLost() const {return _snrLost;};
  void snrLost(float snrLost_) {_snrLost=snrLost_;};
  int holdSearch() const {return _holdSearch;};
  void holdSearch(int holdSearch_) {_holdSearch=holdSearch_;};
  int holdFound() const {return _holdFound;};
  void holdFound(int holdFound_) {_holdFound=holdFound_;};
  int holdLost() const {return _holdLost;}
  void holdLost(int holdLost_){_holdLost=holdLost_;};
  float searchAMin() const {return _searchAMin;};
  void searchAMin(float searchAMin_){_searchAMin=searchAMin_;};
  float searchAMax() const {return _searchAMax;};
  void searchAMax(float searchAMax_){_searchAMax=searchAMax_;};
  float searchStepSize() const {return _searchStepSize;};
  void searchStepSize(float searchStepSize_){_searchStepSize=searchStepSize_;};
  float searchFactor() const {return _searchFactor;};
  void searchFactor(float searchFactor_){_searchFactor=searchFactor_;};
  float maxGD() const {return _maxGD;};
  void maxGD(float maxGD_){_maxGD=maxGD_;};
  float gain() const {return _gain;};
  void gain(float gain_){_gain=gain_;};
private:
  int _state;
  float _snrFound;
  float _snrLost;
  int _holdSearch;
  int _holdFound;
  int _holdLost;
  float _searchAMin;
  float _searchAMax;
  float _searchStepSize;
  float _searchFactor;
  float _maxGD;
  float _gain;
};

class FringeTrackerStateMachineStatistics{
public:
  FringeTrackerStateMachineStatistics(int n=0):counts(n){};
  ~FringeTrackerStateMachineStatistics(){};
  void resize(int n){counts.resize(n);};
  void clear(){for(unsigned int i=0;i<counts.size();i++) counts[i]=0;};
  void update(int);
  std::vector<float> stats();
  virtual int memsize() const {return sizeof(int)+counts.size()*sizeof(int);};
  virtual void write(amjPacket &p) const;
  virtual void read(amjPacket &p);
private:
  std::vector<int> counts;
};

class FringeTrackerStateMachine{
public:
  FringeTrackerStateMachine(std::string name="none", int state=STATE_STOP):
    _name(name),_state(state),
    _stateNames({"Stopped","Searching","Found","Locked","Lost"}){};
  ~FringeTrackerStateMachine(){};
  std::string name(){return _name;};
  std::string stateName() const;
  std::string stateName(int state) const;
  // State functions
  int nstates() const {return _stateNames.size();};
  int state() const {return _state;}; // current state
  void loadDelay(const std::pair<float,float> &d){delay=d.first;snr=d.second;}; // Load the next delay
  int advance(); // Advance the state machine
  float movement() const {return _move;}; // Get the movement

  // Configuration functions
  void setConfig(const FringeTrackerStateMachineConfig &c_){c=c_; _state=c.state();};
  FringeTrackerStateMachineConfig getConfig() {c.state(_state); return c;};
  void reset();
private:
  FringeTrackerStateMachineConfig c; // state machine configuration
  float delay;
  float snr;
  std::string _name; 
  int _state; // Current state
  std::vector<std::string> _stateNames;
  int counter; // counter for hold count
  int searchStep; // size and direction of step in nm
  int cumSearchMove; // Cumulative search movement in nm
  int searchAmplitude; // in nm
  float _move; // Amount to move, in um
};



#endif /* _FRINGETRACKERSTATEMACHINE_H_ */
