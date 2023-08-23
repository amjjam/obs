#include "DelaylineViewer.H"
#include "ui_DelaylineViewer.h"

#include <QNetworkDatagram>
#include <QDateTime>
#include <QVector>

extern int nDelaylines;

DelaylineViewer::DelaylineViewer(QWidget *parent)
  : QMainWindow(parent),ui(new Ui::DelaylineViewer),port(27007){
  ui->setupUi(this);
  ui->chart->ngraphs(nDelaylines);
  delaylinenames.resize(nDelaylines);
  for(int i=0;i<nDelaylines;i++)
    delaylinenames[i]="delayline "+std::to_string(i);
  ui->chart->graphnames(delaylinenames);
  std::cout << "nDelaylines=" << nDelaylines << std::endl;
  std::vector<int> legend_graphs(nDelaylines);
  std::iota(legend_graphs.begin(),legend_graphs.end(),0);
  ui->chart->legendList(legend_graphs);
  ui->chart->legendShow(true);
  connect(ui->spinBox_npanels,QOverload<int>::of(&QSpinBox::valueChanged),ui->chart,QOverload<int>::of(&amjChartView::npanels));
  socket=new QUdpSocket(this);
  socket->bind(QHostAddress::LocalHost,port);
  connect(socket,&QUdpSocket::readyRead,this,&DelaylineViewer::receive);
}

DelaylineViewer::~DelaylineViewer(){
  delete ui;
}

#include <iostream>
void DelaylineViewer::receive(){
  std::cout << "Received" << std::endl;
  while(socket->hasPendingDatagrams()){
    QNetworkDatagram datagram=socket->receiveDatagram();
    packet.clear();
    packet.resize(datagram.data().size());
    memcpy(packet._data(),datagram.data(),datagram.data().size());
    Delays<double> delays;
    packet >> delays;
    ui->chart->ngraphs(delays.size());
    ui->chart->append(QDateTime::currentDateTimeUtc(),QVector<double>(delays.d().begin(),delays.d().end()));
  }
}
