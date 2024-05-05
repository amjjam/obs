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
  template<class U> Delays(const Delays<U> &m){
    _d.resize(m.size());
    for(unsigned int i=0;i<_d.size();i++)
      _d[i]=m[i];
  };
  Delays(int n=0):_d(n,0){};
  virtual ~Delays(){};
  unsigned int size() const {return _d.size();};
  void resize(unsigned int n){_d.resize(n);};
  virtual void write(amjPacket &p) const;
  virtual void read(amjPacket &p);
  std::vector<T> &d(){return _d;}; 
  T &operator[](int i) {return _d[i];};
  const T &operator[](int i) const {return _d[i];};
  template<class U> Delays<T> &operator=(const Delays<U> &);
  template<class U> Delays<T> &operator+=(const Delays<U> &);
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

template<class T>template<class U>
Delays<T> &Delays<T>::operator=(const Delays<U> &m){
  if(size()!=m.size()){
    std::cout << "operator+=: dimensions mismatched: d("
	      << size() << "), m(" << m.size() << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
  
  for(unsigned int i=0;i<size();i++)
    _d[i]=m[i];
  return *this;  
}

template<class T>template<class U>
Delays<T> &Delays<T>::operator+=(const Delays<U> &m){
  if(_d.size()!=m.size()){
    std::cout << "operator+=: dimensions mismatched: d("
	      << _d.size() << "), m(" << m.size() << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
  
  for(unsigned int i=0;i<_d.size();i++)
    _d[i]+=m[i];
  return *this;  
}

template<class T, class U>
Delays<T> operator-(const Delays<T> &p, const Delays<U> &m){
  if(p.size()!=m.size()){
    std::cout << "operator-: dimensions mismatched: p("
	      << p.size() << "), m(" << m.size() << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
  
  Delays<T> r;
  for(unsigned int i=0;i<p.size();i++)
    r[i]=p[i]-m[i];
  return r;  
}

template<class T, class U>
Delays<T> operator+(const Delays<T> &p, const Delays<U> &m){
  if(p.size()!=m.size()){
    std::cerr << "operator+: dimensions mismatched: p("
	      << p.size() << "), m(" << m.size() << ")" << std::endl;
    abort();
  }

  Delays<T> r;
  for(unsigned int i=0;i<p.size();i++)
    r[i]=p[i]+m[i];
  return r;
}

//template<>template<>
//Delays<float> &Delays<float>::operator=(const Delays<double> &);


#endif /* _DELAYS_H_ */

