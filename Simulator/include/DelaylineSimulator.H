#ifndef _DELAYLINESIMULATOR_H_
#define _DELAYLINESIMULATOR_H_

/*============================================================================
  class DelaylineSimulator - this is a simulator of the delay
  lines. It accepts movements (changes in delay), and it will provide
  as output the positions of the delay lines.

  When a movement arrives the time is recorded, the delay is added,
  and and updated position is queued with the delayed time. When a
  position is requested the time is recorded and the most recent
  position in the queue that is less than or equal to the current time
  is returned. As an alternative, a function call can be used to set
  the time. If that function has ever been called then the value of
  the last call is used instead of the current time. This can be
  useful in simulations that run at a different speed then real time.

  void time(struct timespec) - set the time
  void movements(Delays<float>) - a change in delay line positions
  Delays<double> positions() - get the delayline positions
  ============================================================================*/

#include <ctime>


// Delay movements are pushed into this queue as they arrive and used
// to compute the delay line position, with a selectable time-delay. A
// call can be used to get the current delay line positions.
class DelaylineSimulator{
public:
  DelaylineSimulator(int nDelaylines, int timedelay=5000000);
  DelaylineSimulator(Delays<double> initial, struct timespec t={0,0},
		     int time_delay=5000000):timedelay(time_delay){};
  virtual ~DelaylineSimulator(){};
  void time(struct timespec &);
  void movements(Delays<float>);
  Delays<double> positions();
private:
  Delays<double> p;
  long timedelay; // time-delay in nanoseconds (default is 5 ms)
  struct timespect now;
  bool realtime;
  std::queue<std::pair<struct timespec,Delays<double> > > q;
};

inline operator>(struct timespec &, struct timespec &);


#endif
