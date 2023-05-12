#ifndef _DELAYLINESIMULATOR_H_
#define _DELAYLINESIMULATOR_H_

/*============================================================================
  class DelaylineSimulator - this is a simulator of the delay
  lines. It accepts movements (changes in delay), and it will provide
  as output the positions of the delay lines. It incorporates a delay
  between when a movement is received and when it affects the output,
  to simulate transmission delay and any delay in the delay line
  control system.

  When a movement arrives the time is recorded, the time delay is
  added, and and updated position is queued with the delayed
  time. When a position is requested the time is recorded and the most
  recent position in the queue that is less than or equal to the
  current time is returned.

  As an alternative to the object recording the time, a function call
  can be used to set the time. If that function has ever been called
  then the time value provided at the most recent call is used instead
  of recording the current time. This can be useful in simulations
  that run at a different speed then real time.

  (constructor)(int nDelaylines, int timedelay) - set the number of delay
  lines and the time delay in nanoseconds. 
  (constructor)(Delays<double> initial, struct timespect t={0,0}, int timedelay)
  - sets the initial delay line positions and corresponding initial time, and
  time delay in nanoseconds
  void time(struct timespec) - set the time
  void movements(Delays<float>) - a change in delay line positions
  Delays<double> positions() - get the delayline positions
  ============================================================================*/

#include <ctime>
#include <queue>

#include "../../shared/include/Delays.H"

class DelaylineSimulator{
public:
  DelaylineSimulator(int nDelaylines, int timedelay=5000000);
  DelaylineSimulator(Delays<double> initial, struct timespec t={0,0},
		     int time_delay=5000000);
  virtual ~DelaylineSimulator(){};
  void time(struct timespec &);
  void movements(Delays<float> &);
  Delays<double> positions();
private:
  Delays<double> p;
  long timedelay; // time-delay in nanoseconds (default is 5 ms)
  struct timespec now;
  bool realtime;
  std::queue<std::pair<struct timespec,Delays<double> > > q;
};

inline bool operator>(struct timespec &, struct timespec &);


#endif
