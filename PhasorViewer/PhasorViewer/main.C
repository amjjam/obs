#include "phasorviewer.H"

#include <QApplication>

/*==========================================================================
  PhasorViewer [options]

  --receiver-phasors
    communicator specification
  --baseline <name> <n> <L0> <L1>
    string <name> - the name of the baseline
    int <n> - the number of wavelength channels
    float <L0> - the shorter wavelength, in micrometers
    float <L1> - the longer wavelength, in micrometers
  ==========================================================================*/

#include <iostream>

#include <stdlib.h>
#include "../../shared/include/Phasors.H"
std::string receiver_phasors;
std::vector<Wavelengths> wavelengths;
float wavelengthrange[2];

void parse_args(int argc, char *argv[]);
void compute_wavelengthrange();

int main(int argc, char *argv[])
{
  parse_args(argc,argv);
  compute_wavelengthrange();

  QApplication a(argc, argv);
  PhasorViewer w;
  w.show();
  return a.exec();
}

void parse_args(int argc, char *argv[]){
  for(int i=1;i<argc;i++)
    if(strcmp(argv[i],"--receiver-phasors")==0){
      i++;
      receiver_phasors=argv[i];
    }
    else if(strcmp(argv[i],"--baseline")==0){
      wavelengths.push_back(Wavelengths(std::string(argv[i+1]),atoi(argv[i+2]),atof(argv[i+3]),atof(argv[i+4])));
      i+=4;
    }
    else{
      std::cerr << "PhasorViewer: unrecognized option: " << argv[i] << std::endl;
      exit(EXIT_FAILURE);
    }
}

void compute_wavelengthrange(){
  wavelengthrange[0]=1e31; wavelengthrange[1]=-1e31;
  for(unsigned int i=0;i<wavelengths.size();i++){
    if(wavelengths[i].L0()<wavelengthrange[0])
      wavelengthrange[0]=wavelengths[i].L0();
    if(wavelengths[i].L1()>wavelengthrange[1])
      wavelengthrange[1]=wavelengths[i].L1();
  }
}
