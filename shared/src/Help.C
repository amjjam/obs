#include "../include/Help.H"

#include <cstring>
#include <iostream>

void Help::help(int argc, char *argv[]){
  if(argc==1){
    print_help();
    exit(EXIT_SUCCESS);
  }

  for(int i=1;i<argc;i++)
    if(strcmp(argv[i],"-h")==0||strcmp(argv[i],"-help")==0||
       strcmp(argv[i],"--help")==0){
      print_help();
      exit(EXIT_SUCCESS);
    }
}

void Help::print_help(){
  for(unsigned int i=0;i<_help.size();i++)
    std::cout << _help[i] << std::endl;
}
