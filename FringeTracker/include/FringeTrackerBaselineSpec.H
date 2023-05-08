#ifndef _FRINGETRACKERBASELINESPEC_H_
#define _FRINGETRACKERBASELINESPEC_H_

#include "../../shared/include/Wavelengths.H"

class FringeTrackerBaselineSpec{
public:
  FringeTrackerBaselineSpec(Wavelengths L, int nOver, int nIncoherent,
			    int nSmooth):
    _name(L.name()),_L(L),_nOver(nOver),_nIncoherent(nIncoherent),
    _nSmooth(nSmooth){}
  FringeTrackerBaselineSpec(std::string name, int nL, float L0, float L1,
			    int nOver, int nIncoherent, int nSmooth):
    _name(name),_L(name,nL,L0,L1),_nOver(nOver),_nIncoherent(nIncoherent),
    _nSmooth(nSmooth){}
  virtual ~FringeTrackerBaselineSpec(){};
  const std::string &name(){return _name;};
  const Wavelengths &L(){return _L;};
  int nOver(){return _nOver;};
  int nIncoherent(){return _nIncoherent;};
  int nSmooth(){return _nSmooth;};
private:
  std::string _name;
  Wavelengths _L;
  int _nOver;
  int _nIncoherent;
  int _nSmooth;
};


#endif /* _FRINGETRACKERBASELINESPEC_H_ */
