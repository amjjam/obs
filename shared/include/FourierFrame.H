#ifndef _FOURIERFRAME_H_
#define _FOURIERFRAME_H_

#include "Frame.H"
#include "../../shared/include/Wavelengths.H"
#include "../../shared/include/Phasors.H"

template<class T>
class FourierFrame: Frame<T>{
public:
  FourierFrame(unsigned int nL, unsigned int nD);
  FourierFrame(const Wavelengths &, unsigned int nD,
	       const std::vector<Phasors> &, const std::vector<float> &,
	       const std::vector<float> &);
  ~FourierFrame();
  void fill(const Wavelengths &L, const std::vector<Phasors> &p,
	    const std::vector<float> &w, const std::vector<float> &d);
  std::vector<T> &operator[](int iL){return _pixels[iL];};
private:
  std::vector<std::vector<T> > _pixels;
  void init(unsigned int nL, unsigned int nD);
};

template<class T>
FourierFrame::FourierFrame(unsigned int nL, unsigned int nD){
  init(nL,nD);
}

template<class T>
FourierFrame::FourierFrame(const Wavelengths &L, unsigned int nD,
			   const std::vector<Phasors> &p,
			   const std::vector<float> &w,
			   const std::vector<float> &d){
  init(L.size(),nD);
  fill(L,p,w,d);
};

template<class T>
FourierFrame::fill(const Wavelength &L, const std::vector<Phasors> &p,
		   const std::vector<float> &w, const std::vector<float> &d){
  
}

tempalte<class T>
void FourierFrame::init(unsigned int nL, unsigned int nD){
  _pixels.resize(nL);
  for(unsigned int i=0;i<nL;i++)
    _pixels[i].resize(nD);
}

#endif /* _FOURIERFRAME_H_ */
