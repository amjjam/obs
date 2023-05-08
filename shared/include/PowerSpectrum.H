#ifndef _POWERSPECTRUM_H_
#define _POWERSPECTRUM_H_

#include <amjPacket.H>
#include <amjPacketRW.H>

#include <string>
#include <vector>

#include "../../shared/include/Phasors.H"
#include "../../shared/include/Wavelengths.H"

class PowerSpectrumDelays: public amjPacketRW{
public:
  PowerSpectrumDelays(){};
  PowerSpectrumDelays(const Wavelengths &, int);
  virtual unsigned int size() const {return d.size();};
  virtual float operator[](unsigned int i) const {return d[i];};
  virtual void write(amjPacket &p) const {p << d;};
  virtual void read(amjPacket &p){p >> d;};
private:
  std::vector<float> d;
};

class PowerSpectrumPower: public amjPacketRW{
public:
  PowerSpectrumPower(){};
  PowerSpectrumPower(int n, float f=0):p(n,f){};
  virtual ~PowerSpectrumPower(){};
  int size() const {return p.size();};
  void resize(int n){p.resize(n);};
  float &operator[](unsigned int i){return p[i];};
  float operator[](unsigned int i) const {return p[i];};
  void operator+=(PowerSpectrumPower &);
  void operator/=(float);
  virtual void write(amjPacket &p_) const {p_ << p;};
  virtual void read(amjPacket &p_){p_ >> p;};
private:
  std::vector<float> p;
};

class PowerSpectrum: public amjPacketRW{
public:
  PowerSpectrum(){};
  PowerSpectrum(const std::string &name, const PowerSpectrumDelays &delays):
    _name(name),_delays(delays),_power(delays.size(),0){};
  virtual ~PowerSpectrum(){};
  const std::string &name() const {return _name;};
  unsigned int size() const {return _delays.size();};
  void zero(){for(unsigned int i=0;i<_delays.size();i++) _power[i]=0;};
  void fromPhasors(Phasors &p);
  const PowerSpectrumDelays &delays() const {return _delays;};
  PowerSpectrumPower &power() {return _power;};
  virtual void write(amjPacket &p) const {p << _name << _delays << _power;};
  virtual void read(amjPacket &p){p >> _name >> _delays >> _power;};
private:
  std::string _name;
  PowerSpectrumDelays _delays;
  PowerSpectrumPower _power;
  std::vector<float> fftData;
};


amjPacket &operator<<(amjPacket &, std::vector<PowerSpectrum> &);
amjPacket &operator>>(amjPacket &, std::vector<PowerSpectrum> &);

void four1(float data[], unsigned long nn, int isign);

#endif /* _POWERSPECTRUM_H_ */
