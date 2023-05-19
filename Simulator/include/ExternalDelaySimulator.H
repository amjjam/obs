#ifndef _EXTERNALDELAYSIMULATOR_H_
#define _EXTERNALDELAYSIMULATOR_H_

#include "../../shared/include/Delays.H"

#define EXTERNALDELAYSIMULATOR_ZERO 0
#define EXTERNALDELAYSIMULATOR_SIN 1
#define EXTERNALDELAYSIMULATOR_COS 2

class ExternalDelaySimulator{
public:
  ExternalDelaySimulator(int n, struct timespec start={0,0});
  virtual ~ExternalDelaySimulator(){};
  virtual void resize(int n);
  virtual void function(int i, int f, std::vector<double> &p);
  Delays<double> &delays(const struct timespec &t={0,0});
private:
  std::vector<int> _function;
  std::vector<std::vector<double> > params;
  Delays<double> _delays;
  struct timespec start;
  double timediff(struct timespec now);
};

inline bool operator==(const struct timespec &, const struct timespec &);

#endif /* _EXTERNALDELAYSIMULATOR_H_ */
