#include "../include/DelaylineInterfaceSimulator.H"

void DelaylineInterfaceSimulator::move(const Delays<float> &delays){
  if(delays.size()!=ndelaylines){
    std::cout << "DelaylineInterfaceSimulator: size mismatch: ndelaylines="
	      << ndelaylines << ", delays.size()=" << delays.size()
	      << std::endl;
    exit(EXIT_FAILURE);
  }
  p.clear();
  p << delays;
  s.send(p);
  plocal+=delays;
}

void DelaylineInterfaceSimulator::moveto(const Delays<double> &pos){
  if(pos.size()!=ndelaylines){
    std::cout << "DelaylineInterfaceSimulator: size mismatch: ndelaylines="
	      << ndelaylines << ", pos.size()=" << pos.size()
	      << std::endl;
    exit(EXIT_FAILURE);
  }
  Delays<float> delays=pos-plocal;
  p.clear();
  p << delays;
  s.send(p);
  plocal=pos;
}

void DelaylineInterfaceSimulator::setto(const Delays<double> &pos, int i){
  if(pos.size()!=ndelaylines){
    std::cout << "DelaylineInterfaceSimulator: size mismatch: ndelaylines="
	      << ndelaylines << ", pos.size()=" << pos.size()
	      << std::endl;
    exit(EXIT_FAILURE);
  }
  if(i==0)
    plocal=pos;
  else if(i==1)
    pdl=pos;
  else{
    std::cout << "DelaylineSimulator::setto: i=" << i << "out of range 0-1"
	      << std::endl;
    exit(EXIT_FAILURE);
  }
}

Delays<double> DelaylineInterfaceSimulator::pos(int i){
  if(i==0)
    return plocal;
  else if(i==1)
    return pdl;
  else{
    std::cout << "DelaylineSimulator::setto: i=" << i << "out of range 0-1"
	      << std::endl;
    exit(EXIT_FAILURE);
  }
}
