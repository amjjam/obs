#ifndef _SENDERS_H_
#define _SENDERS_H_

#include <string>
#include <ctime>
#include <memory>

#include <amjComUDP.H>

class Senders{
public:
  Senders(){};
  ~Senders(){};
  void addsender(std::string,int);
  void send(amjPacket &p);
private:
  std::vector<std::unique_ptr<amjComEndpointUDP> > endpoints;
  std::vector<int> interval; /* in ms */
  std::vector<struct timespec> last;
  double timediff();
};

#endif /* _SENDERS_H_ */
