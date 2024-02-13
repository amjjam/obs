#ifndef _DELAYS_H_
#define _DELAYS_H_

/*============================================================================
  This class holds a set of delay line positions. This can be absolute
  positions, relative positions, or changes in position to be
  applied. This is a template class, the type of the variables holding
  the delays is the template variable.
  ============================================================================*/

#include <vector>
#include <iostream>

#include <amjCom/amjPacket.H>
#include <amjCom/amjPacketRW.H>

template<class T>
class Delays: public amjPacketRW{
public:
  Delays(int n=0):_d(n,0){};
  virtual ~Delays(){};
  unsigned int size() const {return _d.size();};
  void resize(unsigned int n){_d.resize(n);};
  virtual void write(amjPacket &p) const;
  virtual void read(amjPacket &p);
  std::vector<T> &d(){return _d;}; 
  T &operator[](int i) {return _d[i];};
  const T &operator[](int i) const {return _d[i];};
private:
  std::vector<T> _d;
};
  
template<class T>
void Delays<T>::write(amjPacket &p) const{
  p << _d;
}

template<class T>
void Delays<T>::read(amjPacket &p){
  p >> _d;
}

template<class T>
std::ostream &operator<<(std::ostream &os, const Delays<T> &d){
  for(unsigned int i=0;i<d.size();i++)
    std::cout << d[i] << " ";
  return os;
}

template<class T, class U>
Delays<T> &operator+=(Delays<T> &d, Delays<U> &m){
  if(d.size()!=m.size()){
    std::cout << "operator+=: dimensions mismatched: d("
	      << d.size() << "), m(" << m.size() << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
  
  for(unsigned int i=0;i<d.size();i++)
    d[i]+=m[i];
  return d;  
}

#endif /* _DELAYS_H_ */

