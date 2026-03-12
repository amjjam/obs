#include "powerspectrumviewer.H"
#include "ui_powerspectrumviewer.h"

#include <QString>
#include <iostream>
#include <mutex>

std::mutex mutex_ps, mutex_rate_counter;

PowerSpectrumViewer::PowerSpectrumViewer(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::PowerSpectrumViewer),port(27006),rate_counter(0){
  ui->setupUi(this);
  // socket=new QUdpSocket(this);
  // socket->bind(QHostAddress::LocalHost,port);
  // connect(socket,&QUdpSocket::readyRead,this,&PowerSpectrumViewer::receive);
  connect(ui->lineEdit_server, &QLineEdit::editingFinished, this,
          &PowerSpectrumViewer::server_updated);
  connect(ui->lineEdit_interval, &QLineEdit::editingFinished, this,
          &PowerSpectrumViewer::request_timer_update);
  connect(this, &PowerSpectrumViewer::signal_update_display, this,
          &PowerSpectrumViewer::update_display);
  // Add labels to the status bar
  timeLabel= new QLabel("Date and Time");
  rateLabel= new QLabel("Rate");
  connectionLabel= new QLabel("Connection status");
  statusBar()->addWidget(timeLabel);
  statusBar()->addPermanentWidget(rateLabel);
  statusBar()->addPermanentWidget(connectionLabel);
  // std::cout << "started" << std::endl;
  _colors={Qt::red,Qt::green,Qt::blue,Qt::magenta,Qt::cyan};
  request_timer.callOnTimeout(this, &PowerSpectrumViewer::client_request);
  request_timer_update();
  rate_timer.callOnTimeout(this, &PowerSpectrumViewer::rate_timer_callback);
  rate_timer.start(1000);
}

PowerSpectrumViewer::~PowerSpectrumViewer(){
  delete ui;
}

void PowerSpectrumViewer::server_updated() {
  client.reset();
  client= amjCom::TCP::create_client(
    ui->lineEdit_server->text().toStdString(),
    [this](amjCom::Client c, amjCom::Packet &p) { client_receive(c, p); },
    [this](amjCom::Client c, amjCom::Status s) { client_status(c, s); });
  client->start();
}

void PowerSpectrumViewer::request_timer_update() {
  request_timer.start(ui->lineEdit_interval->text().toInt());
}

void PowerSpectrumViewer::rate_timer_callback() {
  std::lock_guard<std::mutex> lock(mutex_rate_counter);
  rateLabel->setText("rate: " + QString::number(rate_counter) + "/s");
  rate_counter= 0;
}

void PowerSpectrumViewer::client_request() {
  /*std::cout << "client_request: ";*/
  if(!client) {
    /*std::cout << "not sent" << std::endl;*/
    return;
  }
  amjCom::Packet p;
  p.write(1)[0]= 'P';
  client->send(p);
  /*std::cout << "sent" << std::endl;*/
}

void PowerSpectrumViewer::client_receive(amjCom::Client, amjCom::Packet &p) {
  std::lock_guard<std::mutex> lock(mutex_ps);
  uint32_t n;
  memcpy(&n, p.read(sizeof(uint32_t)), sizeof(uint32_t));
  // std::vector<amjInterferometry::PowerSpectrum<float>> ps(n);
  ps.resize(n);
  for(size_t i= 0; i < n; i++) {
    ps[i].read1(p.read(ps[i].memsize1()));
    ps[i].read2(p.read(ps[i].memsize2()));
  }

  QMetaObject::invokeMethod(
    this, [this]() { update_display(); }, Qt::QueuedConnection);

  std::lock_guard<std::mutex> lock2(mutex_rate_counter);
  rate_counter++;
}

void PowerSpectrumViewer::client_status(amjCom::Client, amjCom::Status s) {
  connectionLabel->setText(QString::fromStdString(s.statedescription()));
}

void PowerSpectrumViewer::update_display() {
  std::lock_guard<std::mutex> lock(mutex_ps);
  std::vector<std::string> names;
  for(size_t i= 0; i < ps.size(); i++)
    names.push_back(ps[i].name());
  if(names != _names) {// Plot needs to be reconfigured
    _names= names;
    ui->plot->plotLayout()->clear();
    QCPAxisRect *r= new QCPAxisRect(ui->plot);
    ui->plot->plotLayout()->addElement(0, 0, r);
    for(unsigned int i= 0; i < _names.size(); i++) {
      ui->plot->addGraph();
      ui->plot->graph(i)->setPen(_colors[i % _colors.size()]);
      ui->plot->graph(i)->setName(QString::fromStdString(_names[i]));
      ui->plot->graph(i)->setKeyAxis(
        ui->plot->axisRect(0)->axis(QCPAxis::atBottom, 0));
      ui->plot->graph(i)->setValueAxis(
        ui->plot->axisRect(0)->axis(QCPAxis::atLeft, 0));
    }

    ui->plot->legend= new QCPLegend();
    ui->plot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
    ui->plot->legend->setWrap(5);
    ui->plot->plotLayout()->addElement(ui->plot->axisRectCount(), 0,
                                       ui->plot->legend);
    ui->plot->plotLayout()->setRowStretchFactor(ui->plot->axisRectCount(),
                                                0.00001);
    for(int i= 0; i < ui->plot->graphCount(); i++)
      ui->plot->graph(i)->addToLegend();
    ui->plot->legend->setVisible(true);
  }

  for(unsigned int i= 0; i < ps.size(); i++) {
    QVector<double> d(ps[i].delay().size());
    QVector<double> p(ps[i].power().size());
    for(unsigned int j= 0; j < ps[i].delay().size(); j++) {
      d[j]= ps[i].delay()[j];
      p[j]= ps[i].power()[j];
    }
    ui->plot->graph(i)->setData(d, p);
  }
  amjTime time= ps[0].time();
  QString info= QString("%1/%2/%3 %4:%5:%6")
                  .arg(time.yr())
                  .arg(time.mo())
                  .arg(time.dy())
                  .arg(time.hr(), 2, 10, QChar('0'))
                  .arg(time.mn(), 2, 10, QChar('0'))
                  .arg(time.se(), 2, 10, QChar('0'));
  timeLabel->setText(info);

  ui->plot->yAxis->setTickLabels(false);
  ui->plot->rescaleAxes();
  ui->plot->replot();
}
