#include "../include/DelaylineInterfaceSimulator.H"

void DelaylineInterfaceSimulator::send(const Delays<float> &delays){
  if(delays.size()!=ndelaylines){
    std::cout << "DelaylineInterfaceSimulator: size mismatch: ndelaylines="
	      << ndelaylines << ", delays.size()=" << delays.size()
	      << std::endl;
    exit(EXIT_FAILURE);
  }
  p.clear();
  p << delays;
  s.send(p);
}

