#include "FrameViewer.H"
#include "ui_FrameViewer.h"

#include <QImage>
#include <QPixmap>
#include <QStatusBar>
//#include <QMatrix>

#include <amjCom/amjComMQ.H>
#include <amjCom/amjComTCP.H>
// #include <amjCom/amjPacket.H>
#include <amjFourier.H>

#include <cstdint>
#include <iostream>
#include <algorithm>

Q_DECLARE_METATYPE(amjPacket);

#include <QPainter>
ScaledImageWidget::ScaledImageWidget(QWidget *parent): QWidget(parent) {
  setMinimumSize(1, 1);// allow shrink to tiny sizes
}

void ScaledImageWidget::setImage(const QImage &img) {
  image= img;
  update();// trigger repaint
}

void ScaledImageWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::black);// optional background
  if(!image.isNull()) {
    QImage scaled
        = image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPoint center((width() - scaled.width()) / 2,
                  (height() - scaled.height()) / 2);
    painter.drawImage(center, scaled);
  }
}

FrameViewer::FrameViewer(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::FrameViewer),
    timeDisplay(new amjWidgets::TimeDisplay("Latest: ",this)),
    rateCounter(new amjWidgets::RateCounter("fps:",1000,this)),
    clientStatus(new amjQCom::ClientStatus(this)),seed(0), range(100000),
    scalemin(0), scalemax(range),frame_type('\0'){
  ui->setupUi(this);
  // Add labels to the status bar
  ui->statusBar->addPermanentWidget(rateCounter);
  ui->statusBar->addPermanentWidget(clientStatus);
  ui->statusBar->addWidget(timeDisplay);
  update_scale();
  connect(ui->radioButton_none, &QRadioButton::toggled,
	  this, &FrameViewer::mode_changed);
  connect(ui->radioButton_bias, &QRadioButton::toggled,
	  this, &FrameViewer::mode_changed);
  connect(ui->radioButton_diff, &QRadioButton::toggled,
	  this, &FrameViewer::mode_changed);
  connect(ui->radioButton_raw, &QRadioButton::toggled,
	  this, &FrameViewer::mode_changed);
  connect(ui->radioButton_test, &QRadioButton::toggled,
	  this, &FrameViewer::mode_changed);
  connect(ui->radioButton_sim, &QRadioButton::toggled,
	  this, &FrameViewer::mode_changed);
  connect(ui->lineEdit_cadence, &QLineEdit::editingFinished,
	  this,&FrameViewer::update_timer);
  connect(ui->lineEdit_range, &QLineEdit::editingFinished,
	  this,&FrameViewer::range_updated);
  connect(ui->checkBox_auto, &QCheckBox::checkStateChanged,
	  this,&FrameViewer::auto_clicked);
  connect(ui->lineEdit_server, &QLineEdit::editingFinished,
	  this,&FrameViewer::server_updated);
  connect(ui->verticalSlider_max, &QSlider::valueChanged,
	  this,&FrameViewer::sliderMax_changed);
  connect(ui->verticalSlider_min, &QSlider::valueChanged,
	  this,&FrameViewer::sliderMin_changed);
  connect(this, &FrameViewer::signal_display_frame,
	  this,&FrameViewer::display_frame);
  timer.callOnTimeout(this, &FrameViewer::timer_timeout);
}

#include <QPainter>

FrameViewer::~FrameViewer() { delete ui; }

void FrameViewer::auto_clicked(Qt::CheckState s) {
  if(s == Qt::CheckState::Unchecked)
    update_scale();
  bool enabled= (s == Qt::CheckState::Unchecked);
  ui->lineEdit_range->setEnabled(enabled);
  ui->verticalSlider_min->setEnabled(enabled);
  ui->verticalSlider_max->setEnabled(enabled);
}

void FrameViewer::update_scale() {
  range= round(pow(10, ceil(log10(scalemax))));
  ui->lineEdit_range->setText(QString::number(range));
  ui->verticalSlider_min->setRange(-range, range);
  ui->verticalSlider_max->setRange(-range, range);
  ui->verticalSlider_min->setValue(scalemin);
  ui->verticalSlider_max->setValue(scalemax);
}

void FrameViewer::update_timer() {
  char c;
  {
    std::lock_guard<std::mutex> lock(state_mutex);
    c=frame_type;
  }
  if(!c){
    timer.stop();
    return;
  }
  int period = std::max(1,ui->lineEdit_cadence->text().toInt());
  ui->lineEdit_cadence->setText(QString::number(period));
  timer_timeout(); // to execute immediately when timer is updated
  timer.start(period);
}

void FrameViewer::range_updated() {
  int v= ui->lineEdit_range->text().toInt();
  ui->verticalSlider_min->setRange(-v, v);
  ui->verticalSlider_max->setRange(-v, v);
}

void FrameViewer::server_updated() {
  client.reset();
  request_in_flight=false;
  client=amjCom::TCP::create_client(
    ui->lineEdit_server->text().toStdString(),
    [this](amjCom::Client c, amjCom::Packet &p) {client_receive(c, p);},
    [this](amjCom::Client, amjCom::Status s) {clientStatus->pushStatus(s);});
  client->start();
}

void FrameViewer::client_receive(amjCom::Client, amjCom::Packet &p) {
  {
      std::lock_guard<std::mutex> lock(state_mutex);
      request_in_flight=false;
      char c;
      memcpy(&c,p.read(1),1);
      if(c!=frame_type) // if doesn't match current mode don't display
	return;
  }  
  frame.read1(p.read(frame.size1()));
  frame.read2(p.read(frame.size2()));
  {
    std::lock_guard<std::mutex> lock(frame_mutex);
    iframe=frame;
  }
  for(size_t iL= 0; iL < iframe.wL(); iL++)
    for(size_t iF= 0; iF < iframe.wF(); iF++)
      iframe[iL][iF]=std::clamp(iframe[iL][iF],-100000,100000);
  
  emit signal_display_frame();
}

void FrameViewer::mode_changed(bool checked){
  if (!checked) return;  // only react when button becomes active
  
  char new_type = '\0';
  
  if(ui->radioButton_bias->isChecked()) new_type = 'B';
  else if(ui->radioButton_diff->isChecked()) new_type = 'D';
  else if(ui->radioButton_raw->isChecked())  new_type = 'R';
  else if(ui->radioButton_test->isChecked()) new_type = 'T';
  else if(ui->radioButton_sim->isChecked()) new_type='S';
  
  {
    std::lock_guard<std::mutex> lock(state_mutex);
    frame_type = new_type;
  }
  
  clear_display();
  update_timer();
}

void FrameViewer::timer_timeout(){
  char c;
  {
    std::lock_guard<std::mutex> lock(state_mutex);
    c=frame_type;
  }
  if(!c)
    return;

  if (c=='S') {
    testimage(frame);
    {
      std::lock_guard<std::mutex> lock(frame_mutex);
      iframe=frame;
    }
    emit signal_display_frame();
    return;
  }
  
  if (!client||request_in_flight)
    return;
  
  amjCom::Packet p;
  p.write(1)[0] = c;
  client->send(p);
}

void FrameViewer::sliderMin_changed(int v) {
  ui->label_sliderMin->setText(QString::number(v));
}

void FrameViewer::sliderMax_changed(int v) {
  ui->label_sliderMax->setText(QString::number(v));
}

void FrameViewer::clear_display(){
  {
    std::lock_guard<std::mutex> lock(frame_mutex);
    iframe = amjFourier::Frame<int>();
  }
  ui->image->setImage(QImage());
  ui->label_frameinfo->clear();
}

void FrameViewer::display_frame() {
    std::lock_guard<std::mutex> lock(frame_mutex);
    if (ui->checkBox_auto->isChecked()) {
        struct Histogram histogram;
        framehistogram(iframe, histogram);
        scalemin= histogrampercentile(histogram, 0.01);
        scalemax= histogrampercentile(histogram, 0.99);
    } else {
        scalemin = ui->verticalSlider_min->value();
        scalemax = ui->verticalSlider_max->value();
    }
    QImage image(iframe.wF(), iframe.wL(), QImage::Format_Grayscale8);
    const double scale=255.0/(scalemax-scalemin);
    for(int iH= 0; iH < image.height(); iH++) {
      uchar *line
        = image.scanLine(image.height()-1-iH);// To flip frame vertically
      for(int iW= 0; iW < image.width(); iW++)
	line[iW]=static_cast<uchar>(std::clamp((iframe[iH][iW]-scalemin)*scale,
					       0.0,255.0));
    }
    ui->image->setImage(image);
    rateCounter->count();
    timeDisplay->setTime(iframe.time());
    QString info= QString("(nL,nF,L0,F0,wL,wF)=(%1,%2,%3,%4,%5,%6)")
                    .arg(iframe.nL())
                    .arg(iframe.nF())
                    .arg(iframe.L0())
                    .arg(iframe.F0())
                    .arg(iframe.wL())
                    .arg(iframe.wF());
    ui->label_frameinfo->setText(info);
}

void FrameViewer::framehistogram(const amjFourier::Frame<int> &frame,
                                 struct Histogram &histogram) {
  histogram.bins.assign(200001,0);
  for(size_t iL=0;iL<frame.wL();iL++)
    for(size_t iF=0;iF<frame.wF();iF++)
      histogram.bins[frame[iL][iF]+100000]++;
  histogram.sum=0;
  for(size_t i=0;i<histogram.bins.size();i++)
    histogram.sum+=histogram.bins[i];
}

int FrameViewer::histogrampercentile(const struct Histogram &histogram,
                                     double percentile) {
  uint64_t threshold= percentile * histogram.sum, sum= 0;
  unsigned int value;
  for(value=0;value<histogram.bins.size();value++){
    sum+=histogram.bins[value];
    if(sum>=threshold)
      break;
  }
  return value-100000;
}

void FrameViewer::testimage(amjFourier::Frame<float> &frame) {
  int width= 300, height= 200;
  frame.resize(height, width);
  frame.time(amjTime::Now());
  for(int iL= 0; iL < height; iL++)
    for(int iF= 0; iF < width; iF++)
      frame[iL][iF]= (iF + iL + seed) % 256;
  seed++;
}
