#ifndef _FRINGETRACKERPROTOCOL_H_
#define _FRINGETRACKERPROTOCOL_H_

#define PHASORSAVERAGECONFIG 0x01
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


#define CALCPOWERSPECTRUMCONFIG 0x02
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

#define POWERSPECTRUMAVERAGECONFIG 0x03
struct PowerSpectrumAverageConfig{
  int32_t version;
  enum class AverageType: int16_t{Boxcar=1,LeakyBox=2};
  AverageType avgType;
  double tavg;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};  

#define POWERSPECTRUMSMOOTHCONFIG 0x04
struct PowerSpectrumSmoothingConfig{
  int32_t version;
  enum class SmoothingType: int16_t{None=0,Boxcar=1,Gaussian=2};
  SmoothingType smoothType;
  float width;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define ANALYZEPOWERSPECTRUMCONFIG 0x05
struct AnalyzePowerSpectrumConfig{
  int32_t version;
  float width;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define DELAYFILTERCONFIG 0x06
struct DelayFilterConfig{
  int32_t version;  
  enum class FilterType: int16_t{Boxcar=1,LeakyBox=2,Median=3};
  FilterType filteringType;
  double tfilter;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#define DELAYMACHINECONFIG 0x07
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

#define BASELINENAMECONFIG 0x08
struct BaselineNameConfig{
  int32_t version;
  std::vector<std::string> names;
  uint32_t _size2; // only valid after call to read1()
  size_t size();
  size_t write(uint8_t *);
  size_t size1();
  size_t read1(uint8_t *);
  size_t size2();
  size_t read2(uint8_t *);
};


#define STATEMACHINECONFIG 0x09
struct StateMachineConfig{
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
  uint32_t _size2; // only valid after call to read1()
  size_t size();
  size_t write(uint8_t *);
  size_t size1();
  size_t read1(uint8_t *);
  size_t size2();
  size_t read2(uint8_t *);
};

#define OUTPUTCONFIG 0x0A
struct OutputConfig{
  double Max;
  double Gain;
  int32_t stride;
  size_t size();
  size_t write(uint8_t *);
  size_t read(uint8_t *);
};

#endif
