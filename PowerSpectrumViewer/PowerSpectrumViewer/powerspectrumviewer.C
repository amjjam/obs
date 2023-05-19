#include "powerspectrumviewer.H"
#include "ui_powerspectrumviewer.h"

#include <QNetworkDatagram>
#include <iostream>

PowerSpectrumViewer::PowerSpectrumViewer(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::PowerSpectrumViewer),port(27006)
{
  ui->setupUi(this);
  socket=new QUdpSocket(this);
  socket->bind(QHostAddress::LocalHost,port);
  connect(socket,&QUdpSocket::readyRead,this,&PowerSpectrumViewer::receive);
  std::cout << "started" << std::endl;
  ui->widget->addGraph();
  ui->widget->graph(0)->setPen(QPen(Qt::red));
  ui->widget->addGraph();
  ui->widget->graph(1)->setPen(QPen(Qt::green));
  ui->widget->addGraph();
}

PowerSpectrumViewer::~PowerSpectrumViewer()
{
  delete ui;
}

void PowerSpectrumViewer::receive(){
    while(socket->hasPendingDatagrams()){
      QNetworkDatagram datagram=socket->receiveDatagram();
      packet.clear();
      packet.resize(datagram.data().size());
      memcpy(packet._data(),datagram.data().data(),datagram.data().size());
      std::vector<PowerSpectrum> ps;
      packet >> ps;

      for(unsigned int i=0;i<ps.size();i++){
        QVector<double> d(ps[i].delays().size());
        QVector<double> p(ps[i].power().size());
        for(unsigned int j=0;j<ps[i].delays().size();j++){
            d[j]=ps[i].delays()[j];
            p[j]=ps[i].power()[j];
        }
        ui->widget->graph(i)->setData(d,p);
      }
      ui->widget->xAxis->setRange(0,1024);
      ui->widget->yAxis->setRange(0,50);
      ui->widget->rescaleAxes();
      ui->widget->replot();
    }
}


