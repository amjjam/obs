#ifndef _DATAPROCESSORSTATUS_H_
#define _DATAPROCESSORSTATUS_H_

#include <stdint.h>

#include <amjTime.H>

struct DataProcessorStatus{
  int size();
  void write(uint8_t *d);
  void read(uint8_t *d);
  char state;
  bool hasBias;
  uint32_t nBias;
  amjTime biastime;
  float fps;
};

#endif /* _DATAPROCESSORSTATUS_H_ */

