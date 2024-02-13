#include "DelaylineViewer.H"
#include "ui_DelaylineViewer.h"

#include <QNetworkDatagram>
#include <QDateTime>
#include <QVector>

#include <amjTime.H>

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
  amjTime T;
  while(socket->hasPendingDatagrams()){
    QNetworkDatagram datagram=socket->receiveDatagram();
    packet.clear();
    packet.resize(datagram.data().size());
    memcpy(packet._data(),datagram.data(),datagram.data().size());
    Delays<double> delays;
    T.read(packet.read(T.size()));
    packet >> delays;
    if(delays.size()!=ui->chart->ngraphs())
      ui->chart->ngraphs(delays.size());
    ui->chart->append(QDateTime(QDate(T.yr(),T.mo(),T.dy()),QTime(T.hr(),T.mn(),T.se(),T.ns()/1000000),QTimeZone::utc()),QVector<double>(delays.d().begin(),delays.d().end()));
    std::cout << (int)T.yr() << "/" << (int)T.mo() << "/" << (int)T.dy() << " " << (int)T.hr() << ":" << (int)T.mn() << ":" << (int)T.se() << " " << T.ns() << " ";
    for(int i=0;i<delays.size();i++){
      std::cout << delays[i];
      if(i<delays.size()-1)
        std::cout << ", ";
    }
    std::cout << std::endl;
  }
}
