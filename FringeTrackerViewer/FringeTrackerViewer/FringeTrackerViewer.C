#include "FringeTrackerViewer.H"
#include "ui_FringeTrackerViewer.h"

#include <QNetworkDatagram>
#include <QDateTime>
#include <QVector>

#include <amjChart.H>

FringeTrackerViewer::FringeTrackerViewer(QWidget *parent)
  : QMainWindow(parent),ui(new Ui::FringeTrackerViewer),port(27008),nBaselines(0),nStates(0){
  ui->setupUi(this);
  socket=new QUdpSocket(this);
  socket->bind(QHostAddress::LocalHost,port);
  connect(socket,&QUdpSocket::readyRead,this,&FringeTrackerViewer::receive);

  ui->chart->colors(QVector<QColor>{Qt::black,Qt::red,Qt::yellow,Qt::green,Qt::blue});
}

FringeTrackerViewer::~FringeTrackerViewer(){
  delete ui;
}

#include <iostream>
void FringeTrackerViewer::receive(){
  //std::cout << "Received" << std::endl;
  std::vector<std::string> baselinenames,statenames;
  std::vector<float> d;
  QVector<double> data;
  while(socket->hasPendingDatagrams()){
    QNetworkDatagram datagram=socket->receiveDatagram();
    packet.clear();
    packet.resize(datagram.data().size());
    memcpy(packet._data(),datagram.data(),datagram.data().size());
    packet >> baselinenames >> statenames;
    if(baselinenames!=_baselinenames||statenames!=_statenames){
      ui->chart->clear();
      nBaselines=baselinenames.size();
      nStates=statenames.size();
      ui->chart->npanels(nBaselines);
      ui->chart->ngraphs(nBaselines*nStates);
      ui->chart->panelnames(baselinenames);
      ui->chart->graphnames(statenames);
      std::vector<int> legend_graphs(statenames.size());
      std::iota(legend_graphs.begin(),legend_graphs.end(),0);
      ui->chart->legendList(legend_graphs);
      ui->chart->legendShow(true);
      ui->chart->legendWrap(nStates);
      _baselinenames=baselinenames;
      _statenames=statenames;
    }
    data.clear();
    for(int i=0;i<nBaselines;i++){
      //std::cout << "reading: " << i << " of " << nBaselines << std::endl;
      packet >> d;
      for(unsigned int i=1;i<d.size();i++)
        d[i]+=d[i-1];
      //std::cout << d.size() << std::endl;
      for(unsigned j=0;j<d.size();j++)
        d[j]*=100;
      data << QVector<double>(d.begin(),d.end());
    }
    ui->chart->append(QDateTime::currentDateTimeUtc(),data);
  }
}
