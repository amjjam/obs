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
  //std::cout << "started" << std::endl;
  _colors={Qt::red,Qt::green,Qt::blue,Qt::magenta,Qt::cyan};
}

PowerSpectrumViewer::~PowerSpectrumViewer(){
  delete ui;
}

void PowerSpectrumViewer::receive(){
    while(socket->hasPendingDatagrams()){
      QNetworkDatagram datagram=socket->receiveDatagram();
      packet.clear();
      packet.resize(datagram.data().size());
      memcpy(packet._data(),datagram.data().data(),datagram.data().size());
      std::vector<std::string> names;
      std::vector<PowerSpectrum> ps;
      packet >> names;
      packet >> ps;

      if(names!=_names){ // Plot needs to be reconfigured
        _names=names;
        std::cout << "reconfigure" << std::endl;
        ui->plot->plotLayout()->clear();
        QCPAxisRect *r=new QCPAxisRect(ui->plot);
        ui->plot->plotLayout()->addElement(0,0,r);
        for(unsigned int i=0;i<_names.size();i++){
          ui->plot->addGraph();
          ui->plot->graph(i)->setPen(_colors[i%_colors.size()]);
          ui->plot->graph(i)->setName(QString::fromStdString(_names[i]));
          ui->plot->graph(i)->setKeyAxis(ui->plot->axisRect(0)->axis(QCPAxis::atBottom,0));
          ui->plot->graph(i)->setValueAxis(ui->plot->axisRect(0)->axis(QCPAxis::atLeft,0));
        }

        ui->plot->legend=new QCPLegend();
        ui->plot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
        ui->plot->legend->setWrap(5);
        ui->plot->plotLayout()->addElement(ui->plot->axisRectCount(),0,ui->plot->legend);
        ui->plot->plotLayout()->setRowStretchFactor(ui->plot->axisRectCount(),0.00001);
        for(int i=0;i<ui->plot->graphCount();i++)
          ui->plot->graph(i)->addToLegend();
        ui->plot->legend->setVisible(true);
      }

      for(unsigned int i=0;i<ps.size();i++){
        QVector<double> d(ps[i].delays().size());
        QVector<double> p(ps[i].power().size());
        for(unsigned int j=0;j<ps[i].delays().size();j++){
            d[j]=ps[i].delays()[j];
            p[j]=ps[i].power()[j];
        }
        std::cout << "setdata" << std::endl;
        ui->plot->graph(i)->setData(d,p);
      }
      ui->plot->xAxis->setRange(0,1024);
      ui->plot->yAxis->setRange(0,50);
      ui->plot->rescaleAxes();
      ui->plot->replot();
    }
}


