#include "phasorviewer.H"
#include "ui_phasorviewer.h"

#include <QNetworkDatagram>
#include <QString>
#include <QColor>

#include <iostream>

#include <cmath>

#include <amjTime.H>

extern std::string receiver_phasors;
extern std::vector<Wavelengths> wavelengths;
extern float wavelengthrange[2];

std::vector<QColor> colors={QColor(255,0,0),QColor(0,255,0),QColor(0,0,255),QColor(0,255,255),
                            QColor(255,0,255),QColor(255,255,0)};

PhasorViewer::PhasorViewer(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::PhasorViewer)
{
  ui->setupUi(this);
  socket=new QUdpSocket(this);
  std::cout << receiver_phasors << std::endl;
  if(QString::fromStdString(receiver_phasors).indexOf(':')>=0){
    socket->bind(QHostAddress(QString::fromStdString(receiver_phasors).split(':')[0]),QString::fromStdString(receiver_phasors).split(':')[1].toInt());
    connect(socket,&QUdpSocket::readyRead,this,&PhasorViewer::receive);
    std::cout << "binding" << std::endl;
  }

  // Create the plots
  ui->plot->plotLayout()->clear();
  v2plot=new QCPAxisRect(ui->plot);
  phiplot=new QCPAxisRect(ui->plot);
  ui->plot->plotLayout()->addElement(0,0,v2plot);
  v2plot->axis(QCPAxis::atBottom,0)->setRange(wavelengthrange[0],wavelengthrange[1]);
  v2plot->axis(QCPAxis::atBottom,0)->setLabel("Wavelength (micrometers)");
  v2plot->axis(QCPAxis::atLeft,0)->setLabel("Amplitude");
  ui->plot->plotLayout()->addElement(1,0,phiplot);
  phiplot->axis(QCPAxis::atBottom,0)->setRange(wavelengthrange[0],wavelengthrange[1]);
  phiplot->axis(QCPAxis::atBottom,0)->setLabel("Wavelength (micrometers)");
  phiplot->axis(QCPAxis::atLeft,0)->setRange(-180,180);
  phiplot->axis(QCPAxis::atLeft,0)->setLabel("Phase (degrees)");

  // Create the graphs on the plots
  ui->plot->clearGraphs(); // Since we are just starting there should be no graphs so this is superfluous
  for(unsigned int i=0;i<wavelengths.size();i++){
      ui->plot->addGraph(v2plot->axis(QCPAxis::atBottom),v2plot->axis(QCPAxis::atLeft))->setPen(colors[i]);
      ui->plot->addGraph(phiplot->axis(QCPAxis::atBottom),phiplot->axis(QCPAxis::atLeft))->setPen(colors[i]);
    }
}


PhasorViewer::~PhasorViewer()
{
  delete ui;
}

void PhasorViewer::receive(){
  while(socket->hasPendingDatagrams()){
      QNetworkDatagram datagram=socket->receiveDatagram();
      packet.resize(datagram.data().size());
      memcpy(packet.raw(),datagram.data().data(),datagram.data().size());
      packet.reset();
      amjTime T;
      PhasorSets ps;
      T.read(packet.read(T.size()));
      packet >> ps;
      update_plots(ps);
    }
}

void PhasorViewer::update_plots(PhasorSets &p){
  // set the data for each graph one at a time
  QVector<double> v2,phi,w;
  for(unsigned int i=0;i<p.size();i++){
      v2.resize(p[i].size());
      phi.resize(p[i].size());
      w.resize(p[i].size());
      for(unsigned int j=0;j<p[i].size();j++){
          v2[j]=std::fabs(p[i][j]);
          phi[j]=std::arg(p[i][j])/M_PI*180;
          w[j]=wavelengths[i][j];
        }
      ui->plot->graph(2*i)->setData(w,v2);
      ui->plot->graph(2*i+1)->setData(w,phi);
    }
  ui->plot->rescaleAxes();
  ui->plot->replot();
}
