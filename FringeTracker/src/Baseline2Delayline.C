#include "../include/Baseline2Delayline.H"

Baseline2Delayline::Baseline2Delayline(int nB, int nD){
  B.resize(nB);
  D.resize(nD);
}

Baseline2Delayline::~Baseline2Delayline(){}

#include <iostream>
void Baseline2Delayline::loadBaselineMovements(Delays<float> &b){
  if(B.size()!=b.size()){
    std::cout << "Baseline2Delayline::loadBaselineMovement: B(" << B.size()
	      << ") does not match b(" << b.size() << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
  B=b;
}
