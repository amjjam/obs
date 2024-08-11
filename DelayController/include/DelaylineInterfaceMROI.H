#ifndef _DELAYLINEINTERFACEMROI_H_
#define _DELAYLINEINTERFACEMROI_H_

#include <mutex>

#include <amjCom/amjComUDP.H>

#include "DelaylineInterface.H"

#define MAX_DLS 10
#define DECIMATION 1
#define SAMPLES_PER_CHUNK 10  /* max. 13 for standard payload size */
#define DLPOS_PAD ((SAMPLES_PER_CHUNK * 2) - (((SAMPLES_PER_CHUNK * 2)/8) * 8))

typedef struct{
  int8_t isValid; /**< Are the command parameters valid? */
  double velocity; /**< V parameter [m.s-1] */
  int64_t epoch; /**< T parameter */
  double offset; /**< C parameter [m] */
  int8_t isSearching; /**< Is this DL involved in a fringe search? */
  int8_t _pad[6]; /**< Padding for consistency between 32 bit and 64 bit */
} FringeOffset;

typedef struct{
  int64_t epoch;    /**< Measurement epoch for position[0] */
  int64_t interval; /**< Nominal delta-epoch for position[] */
  double position[SAMPLES_PER_CHUNK];   /**< Delay line position [m] */
  int8_t isTracking[SAMPLES_PER_CHUNK]; /**< Is DL in track mode? */
  int8_t isValid[SAMPLES_PER_CHUNK];    /**< Is position valid? */
#if DLPOS_PAD != 0
  int8_t _pad[DLPOS_PAD]; /**< Padding for consistency between 32 bit and 64 bit */
#endif
} DelayLinePosition;

class DelaylineInterfaceMROI:public DelaylineInterface<float,double>{
public:
  DelaylineInterfaceMROI(unsigned int ndelaylines, std::string sender,
			 std::string receiver);
  ~DelaylineInterfaceMROI(){};
  virtual void move(const Delays<float> &);
  virtual void moveto(const Delays<double> &);
  virtual void setto(const Delays<double> &, int i=0);
  virtual int nsets(){return 2;};
  virtual Delays<double> pos(int i=0);
private:
  std::mutex m;
  void send();
  void receive();
  template<class T>
  void check_dimensions(const Delays<T> &);
  unsigned int ndelaylines;
  Delays<double> plocal;
  Delays<double> pdl;
  amjComEndpointUDP c;
  amjPacket ps;
  amjPacket pr;
  FringeOffset offsets[MAX_DLS];
  DelayLinePosition positions[MAX_DLS];
};

#endif /* _DELAYLINEINTERFACEMROI_H_ */

