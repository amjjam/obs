#ifndef _TIMING_H_
#define _TIMING_H_

#include <time.h>

class Timing: public std::vector<struct timespec>{
public:
  Timing(int n){resize(n);};
  virtual ~Timing(){};
  struct timespec &time(std::vector<struct timespec>::size_type i)
  {clock_gettime(CLOCK_MONOTONIC,&(*this)[i]); return (*this)[i];};
private:
};

class TimingStats: std::vector<float>{
public:
  TimingStats(int n){};
  void accum(Timing &){};
  void stats(){};
private:
  std::vector<Timing> _timing;
  std::vector<std::string> name;
  std::vector<float> min;
  std::vector<float> mean;
  std::vector<float> max;
};

#endif /* _TIMING_H_ */
