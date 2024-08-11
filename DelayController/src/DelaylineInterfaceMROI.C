#include "../include/DelaylineInterfaceMROI.H"

DelaylineInterfaceMROI::DelaylineInterfaceMROI(unsigned int ndelaylines,
					       std::string sender,
					       std::string receiver):
  ndelaylines(ndelaylines),plocal(ndelaylines),pdl(ndelaylines),
  c(receiver,sender){
  if(ndelaylines>MAX_DLS){
    std::cout << "DelaylineInterfaceMROI: ndelaylines=" << ndelaylines
	      << " larger than MAX_DLS=" << MAX_DLS << std::endl;
    exit(EXIT_FAILURE);
  }

  for(unsigned int i=0;i<MAX_DLS;i++){
    offsets[i]={0,0,0,0,0,{0,0,0,0,0,0}};
    if(i<ndelaylines)
      offsets[i].isValid=1;
  }
  
  /* .isValid=1, .epoch=0 or .epoch=now?, what about .isSearching? */

  /* start a receiver thread */

};

void DelaylineInterfaceMROI::move(const Delays<float> &delays){
  check_dimensions<float>(delays);
  plocal+=delays;
  send();
}

void DelaylineInterfaceMROI::moveto(const Delays<double> &delays){
  check_dimensions<double>(delays);
  plocal=delays;
  send();
}

void DelaylineInterfaceMROI::setto(const Delays<double> &delays, int i){
  if(i==0)
    plocal=delays;
  else if(i==1){
    m.lock();
    pdl=delays;
    m.unlock();
  }
  else{
    std::cerr << "DelaylineInterfaceMROI::setto: i=" << i << std::endl;
    exit(EXIT_FAILURE);
  }
}

Delays<double> DelaylineInterfaceMROI::pos(int i){
  if(i==0)
    return plocal;
  else if(i==1){
    m.lock();
    Delays<double> xpdl=pdl;
    m.unlock();
    return xpdl;
  }
  else{
    std::cerr << "DelaylineInterfaceMROI::pos: i=" << i << std::endl;
    exit(EXIT_FAILURE);
  }
}

void DelaylineInterfaceMROI::send(){
  for(unsigned int i=0;i<ndelaylines;i++)
    offsets[i].offset=plocal[i]/1e6;
  ps.start();
  memcpy(ps.write(sizeof(FringeOffset)*MAX_DLS),&offsets,
	 sizeof(FringeOffset)*MAX_DLS);
  c.send(ps);
}

void DelaylineInterfaceMROI::receive(){
  c.receive(pr);
  memcpy(positions,pr.read(sizeof(DelayLinePosition)*MAX_DLS),
	 sizeof(DelayLinePosition)*MAX_DLS);
  m.lock();
  for(unsigned int i=0;i<ndelaylines;i++)
    pdl[i]=positions[i].position[0]*1e6;
  m.unlock();
}

template<class T>
void DelaylineInterfaceMROI::check_dimensions(const Delays<T> &delays){
  if(delays.size()!=ndelaylines){
    std::cout << "DelaylineInterfaceSimulator: size mismatch: ndelaylines="
	      << ndelaylines << ", delays.size()=" << delays.size()
	      << std::endl;
    exit(EXIT_FAILURE);
  }
}
