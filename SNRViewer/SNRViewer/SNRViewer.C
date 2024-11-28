#include "SNRViewer.H"
#include "ui_SNRViewer.h"

#include <QNetworkDatagram>
#include <QDateTime>
#include <QVector>

#include <amjTime.H>

#include <iostream>

extern int nBaselines;

SNRViewer::SNRViewer(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::SNRViewer),port(27011){
  ui->setupUi(this);
  ui->chart->ngraphs(nBaselines);
  baselinenames.resize(nBaselines);
  for(int i=0;i<nBaselines;i++)
    baselinenames[i]="baseline "+std::to_string(i);
  ui->chart->graphnames(baselinenames);
  std::cout << "nBaselines=" << nBaselines << std::endl;
  std::vector<int> legend_graphs(nBaselines);
  std::iota(legend_graphs.begin(),legend_graphs.end(),0);
  ui->chart->legendList(legend_graphs);
  ui->chart->legendWrap(3);
  ui->chart->legendShow(true);
  connect(ui->spinBox_npanels,QOverload<int>::of(&QSpinBox::valueChanged),ui->chart,QOverload<int>::of(&amjChartView::npanels));
  socket=new QUdpSocket(this);
  socket->bind(QHostAddress::LocalHost,port);
  connect(socket,&QUdpSocket::readyRead,this,&SNRViewer::receive);
}

SNRViewer::~SNRViewer()
{
  delete ui;
}

void SNRViewer::receive(){
  std::cout << "Received" << std::endl;
  int nValues;
  std::vector<float> values;
  amjTime T;
  while(socket->hasPendingDatagrams()){
      QNetworkDatagram datagram=socket->receiveDatagram();
      std::cout << "datagram.size()=" << datagram.data().size() << std::endl;
      packet.clear();
      packet.resize(datagram.data().size());
      memcpy(packet._data(),datagram.data(),datagram.data().size());
      T.read(packet.read(T.size()));
      memcpy(&nValues,packet.read(sizeof(int)),sizeof(int));
      if(nValues!=nBaselines){
          std::cout << "SNRViewer: receive: error: nValues=" << nValues << ", nBaselines=" << nBaselines << std::endl;
          continue;
      }
      values.resize(nValues);
      memcpy(&values[0],packet.read(nValues*sizeof(float)),nValues*sizeof(float));
      ui->chart->append(QDateTime(QDate(T.yr(),T.mo(),T.dy()),QTime(T.hr(),T.mn(),T.se(),T.ns()/1000000),QTimeZone::utc()),QVector<double>(values.begin(),values.end()));
  }
}
