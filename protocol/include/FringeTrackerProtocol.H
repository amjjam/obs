#ifndef _FRINGETRACKERPROTOCOL_H_
#define _FRINGETRACKERPROTOCOL_H_

/*============================================================================
  Communication between FringeTracker and other programs.
  FringeTracker never initiates communication, it only listens, and
  always responds.

  The first character of a packet from FringeTrackerGUI is always
  G. This is followed by the phasors version number. If the phasors
  version number does not match, FringeTracker will send back B
  followed by a struct BaselineNameConfig, and ignore the rest of the
  packet.

  When the FringeTrackerGUI receives B it will update the baseline
  names and the version number, and send back a new packet to
  FringeTracker containing complete configuration settings.

  When FringeTracker receives a packet matching the current phasors
  version number, it will execute the packet contents and then respond
  with a packet U containing amjTime (of most recent phasors received,
  or current time if no phasors received), followed by uint32_t frame
  rate.

  Format of packet from FringeTrackerGUI to FringeTracker:

  uint8_t 'G'
  int32_t version // pversion
  Then follow zero or more sections, terminated with a null character '\0'
  Each section is one character followed by the corresponding structure defined
  below

  '1' FringeTrackerCommand
  '2' PhasorsAverageConfig
  '3' CalcPowerSpectrumConfig
  '4' PowerSpectrumAverageConfig
  '5' PowerSpectrumSmoothConfig
  '6' AnalyzePowerSpectrumConfig
  '7' DelayFilterConfig
  '8' StateMachineConfig
  '9' OutputConfig

  DelayMachineConfig and StateMachineConfig are composed of
  sub-structures. Each time FringeTrackerGUI changes something in a
  structure or a sub-structure, it will increment the version number
  in that structure as well as the version number in its parent
  structure.

  When FringeTracker receives a packet it does.
  Read first character. If it is 'G' then lock configuration with mutex_config.
  Read version number
  If it does not match pversion return packet of new baselines names
  If it matches pversion execute rest of packet:
    If a FC section is found, update runstate
    If a PA section is found, update phasorsaverageconfig
    If a DM section is found, update delaymachineconfig
    If a SM section is found, update statemachineconfig
    If a OP section is found, update outputconfig
    If '\0\0' is found, end processing
  ===========================================================================*/

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#define FRINGETRACKERCOMMAND 0x01
struct FringeTrackerCommand{
  enum class CommandType: uint8_t {Start=1,Stop=2};
  uint8_t cmd;
};

#define PHASORSAVERAGECONFIG 0x02
struct PhasorsAverageConfig{
  int32_t version;
  enum class AverageType: uint8_t{Boxcar=1,LeakyBox=2};
  AverageType avgType;
  float tavg;
  int32_t stride;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};


#define CALCPOWERSPECTRUMCONFIG 0x03
struct CalcPowerSpectrumConfig{
  int32_t version;
  enum class CalcType: int16_t {FFT=1,DFT=2};
  CalcType calcType;
  float dftMin;
  float dftMax;
  float dftStep;
  int16_t nOver;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define POWERSPECTRUMAVERAGECONFIG 0x04
struct PowerSpectrumAverageConfig{
  int32_t version;
  enum class AverageType: int16_t{Boxcar=1,LeakyBox=2};
  AverageType avgType;
  double tavg;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};  

#define POWERSPECTRUMSMOOTHCONFIG 0x05
struct PowerSpectrumSmoothingConfig{
  int32_t version;
  enum class SmoothingType: int16_t{None=0,Boxcar=1,Gaussian=2};
  SmoothingType smoothType;
  float width;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define ANALYZEPOWERSPECTRUMCONFIG 0x06
struct AnalyzePowerSpectrumConfig{
  int32_t version;
  float width;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define DELAYFILTERCONFIG 0x07
struct DelayFilterConfig{
  int32_t version;  
  enum class FilterType: int16_t{None=0,Boxcar=1,LeakyBox=2,Median=3};
  FilterType filteringType;
  double tfilter;
  int32_t stride;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define DELAYMACHINECONFIG 0x08
struct DelayMachineConfig{
  CalcPowerSpectrumConfig cp;
  PowerSpectrumAverageConfig pa;
  PowerSpectrumSmoothingConfig ps;
  AnalyzePowerSpectrumConfig ap;
  DelayFilterConfig df;
  int32_t version;
  int32_t stride;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define BASELINENAMECONFIG 0x09
struct BaselineNameConfig{
  int32_t version; // pversion
  std::vector<std::string> names;
  size_t size();
  size_t write(uint8_t *);
  size_t size1();
  size_t read1(uint8_t *);
  size_t size2();
  size_t read2(uint8_t *);
private:
  uint32_t _size2; // only valid after call to read1()
};


#define STATEMACHINESCONFIG 0x0A
struct StateMachinesConfig{
  struct BaselineConfig{
    float SNRFound;
    float SNRLost;
    int32_t TFound;
    int32_t TLost;
  };
  struct SearchConfig{
    int32_t hold;
    double Amin;
    double Amax;
    double step;
    double factor;
  };
  int32_t version;
  std::vector<BaselineConfig> bc;
  SearchConfig sc;
  int32_t stride;
  size_t size();
  size_t write(uint8_t *);
  size_t size1();
  size_t read1(uint8_t *);
  size_t size2();
  size_t read2(uint8_t *);
private:
  uint32_t _size2; // only valid after call to read1()
};

#define OUTPUTCONFIG 0x0B
struct OutputConfig{
  double Max;
  double Gain;
  int32_t stride;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#endif
