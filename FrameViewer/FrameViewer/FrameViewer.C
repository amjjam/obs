#include "FrameViewer.H"
#include "ui_FrameViewer.h"

#include <QImage>
#include <QPixmap>
//#include <QMatrix>

#include <amjCom/amjComMQ.H>
#include <amjCom/amjPacket.H>
#include <amjFourier.H>

#include <iostream>
#include <cstdint>

Q_DECLARE_METATYPE(amjPacket);

FrameViewer::FrameViewer(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::FrameViewer){
  ui->setupUi(this);
  receiver=new Receiver();
  connect(receiver,&Receiver::received,this,&FrameViewer::receive);
  qRegisterMetaType<amjPacket>();
  receiver->start();
}

FrameViewer::~FrameViewer(){
  delete ui;
}

void FrameViewer::receive(amjPacket p){
  std::cout << "Frameviewer::receive" << std::endl;
  receiver->start();
  Frame<uint16_t> frame(64,64);
  p >> frame;
  QImage image(64,64,QImage::Format_RGB888);
  int sum=0;
  QColor c;//=QColor(frame[iL][iF],frame[iL][iF],frame[iL][iF]);
  for(unsigned int iL=0;iL<frame.nL();iL++)
    for(unsigned int iF=0;iF<frame.nF();iF++){
      c.setRgb(frame[iL][iF],frame[iL][iF],frame[iL][iF]);
      image.setPixelColor(iL,iF,c);
      sum+=frame[iL][iF];
    }
  QImage imagerotated=image.transformed(QMatrix().rotate(90.0));
  ui->image->setPixmap(QPixmap::fromImage(imagerotated));
  std::cout << sum << std::endl;
}

void Receiver::run(){
  amjComEndpointMQ r("/frameviewer:2:10000","");
  amjPacket p;
  r.receive(p);
  std::cout << "got one" << std::endl;
  emit received(p);
}
