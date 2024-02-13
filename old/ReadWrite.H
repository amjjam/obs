#ifndef _READWRITE_H_
#define _READWRITE_H_

#include <amjPacket.H>

class ReadWrite{
public:
  ReadWrite(){};
  virtual ~ReadWrite()=0;
  virtual int write(amjPacket &, unsigned int offset=0) const =0;
  virtual int read(const amjPacket &, unsigned int offset=0)=0;
private:
};


#endif /* _READWRITE_H_ */
