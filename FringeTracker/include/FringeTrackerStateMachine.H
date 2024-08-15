#ifndef _FRINGETRACKERSTATEMACHINE_H_
#define _FRINGETRACKERSTATEMACHINE_H_

#include <string>
#include <vector>
#include <complex>

#include <amjCom/amjPacketRW.H>

#define OFF 0
#define ON 1

#define STATE_STOP 0
#define STATE_SEARCH 1
#define STATE_FOUND 2
#define STATE_LOCK 3
#define STATE_LOST 4

class FringeTrackerStateMachineConfig{
public:
  FringeTrackerStateMachineConfig(int onoff=OFF, float foundSNR=6,
				  float lostSNR=3, int holdSearch=10,
				  int holdFound=3, int holdLost=10,
				  float searchAMin=3,
				  float searchAMax=5000,
				  float step=3, float factor=1.5,
				  float maxGD=10, float gain=0.1);
  ~FringeTrackerStateMachineConfig(){};
  int onoff;
  float snrFound;
  float snrLost;
  int holdSearch;
  int holdFound;
  int holdLost;
  float searchAMin;
  float searchAMax;
  float searchStepSize;
  float searchFactor;
  float maxGD;
  float gain;
  int size();
  int write(uint8_t *);
  int read(uint8_t *);
  bool operator==(const FringeTrackerStateMachineConfig &) const;
  float rdiff(float a, float b) const {return fabs(a-b)/(a+b);};
  void print();
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
  void loadDelay(const std::pair<float,float> &d){delay=d.first;_snr=d.second;};// Load the next delay
  float snr(){return _snr;};
  int advance(); // Advance the state machine
  float movement() const {return _move;}; // Get the movement

  // Configuration functions
  void setConfig(const FringeTrackerStateMachineConfig &c_){c=c_;};
  const FringeTrackerStateMachineConfig &getConfig() {return c;};
  void reset();
private:
  FringeTrackerStateMachineConfig c; // state machine configuration
  float delay;
  float _snr;
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
