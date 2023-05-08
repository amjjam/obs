#ifndef _WAVELENGTHS_H_
#define _WAVELENGTHS_H_

#include <string>
#include <vector>

class Wavelengths{
public:
  Wavelengths(){};
  Wavelengths(std::string,int,float,float);
  Wavelengths(std::string, std::vector<float> &);
  virtual ~Wavelengths(){};
  long unsigned int size() const {return _L.size();};
  const std::string &name() const {return _name;};
  const std::vector<float> &wavelengths(){return _L;};
  float operator[](int i) const {return _L[i];};
  float L0() const {return _L0;};
  float L1() const {return _L1;};
private:
  std::string _name;
  float _L0,_L1;
  std::vector<float> _L;
};

#endif /* _WAVELENGTHS_H_ */
