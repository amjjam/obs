#include "FrameViewer.H"
#include "ui_FrameViewer.h"

#include <QImage>
#include <QPixmap>
//#include <QMatrix>

#include <amjCom/amjComMQ.H>
#include <amjCom/amjComTCP.H>
// #include <amjCom/amjPacket.H>
#include <amjFourier.H>

#include <cstdint>
#include <iostream>

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
    : QMainWindow(parent), ui(new Ui::FrameViewer), seed(0), range(100000),
      scalemin(0), scalemax(range), fps_counter(0) {
  ui->setupUi(this);
  update_scale();
  connect(ui->radioButton_none,
          &QRadioButton::toggled,
          this,
          &FrameViewer::radio_none_or_sim_toggled);
  connect(ui->radioButton_sim,
          &QRadioButton::toggled,
          this,
          &FrameViewer::radio_none_or_sim_toggled);
  connect(ui->lineEdit_cadence, &QLineEdit::editingFinished, this,&FrameViewer::update_timer);
  connect(ui->lineEdit_range, &QLineEdit::editingFinished, this,&FrameViewer::range_updated);
  connect(ui->checkBox_auto, &QCheckBox::checkStateChanged, this,&FrameViewer::auto_clicked);
  connect(ui->lineEdit_server, &QLineEdit::editingFinished, this,&FrameViewer::server_updated);
  connect(ui->verticalSlider_max, &QSlider::valueChanged, this,
          &FrameViewer::sliderMax_changed);
  connect(ui->verticalSlider_min, &QSlider::valueChanged, this,
          &FrameViewer::sliderMin_changed);
  send_timer.callOnTimeout(this, &FrameViewer::send_timer_timeout);
  fps_timer.callOnTimeout(this, &FrameViewer::fps_timer_timeout);
  fps_timer.start(1000);
}

#include <QPainter>

FrameViewer::~FrameViewer() { delete ui; }

void FrameViewer::radio_none_or_sim_toggled(bool){
    update_timer();
}

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
    if (ui->radioButton_none->isChecked())
        send_timer.stop();
    else {
        int period = ui->lineEdit_cadence->text().toInt();
        if (period < 1) {
            period = 1;
            ui->lineEdit_cadence->setText(QString::number(period));
        }
        send_timer.start(ui->lineEdit_cadence->text().toInt());
    }
}

void FrameViewer::range_updated() {
  int v= ui->lineEdit_range->text().toInt();
  ui->verticalSlider_min->setRange(-v, v);
  ui->verticalSlider_max->setRange(-v, v);
}

void FrameViewer::server_updated() {
  client.reset();
  client= amjCom::TCP::create_client(
      ui->lineEdit_server->text().toStdString(),
      [this](amjCom::Client c, amjCom::Packet &p) { client_receive(c, p); },
      [this](amjCom::Client c, amjCom::Status s) { client_status(c, s); });
  client->start();
}

std::mutex frame_mutex;
void FrameViewer::client_receive(amjCom::Client, amjCom::Packet &p) {
  std::lock_guard<std::mutex> lock(frame_mutex);

  p.begin();

  if(p.size() < 100000) {
    std::cout << "client_receive: short packet: size=" << p.size()
              << std::endl;
    return;
  }

  if(ui->radioButton_sim->isChecked()) {
    client.reset();
    return;
  }

  frame.read1(p.read(frame.size1()));
  frame.read2(p.read(frame.size2()));

  iframe= frame;
  for(size_t iL= 0; iL < iframe.wL(); iL++)
    for(size_t iF= 0; iF < iframe.wF(); iF++) {
      int v= iframe[iL][iF];
      v= v < -100000 ? -100000 : v;
      v= v > 100000 ? 100000 : v;
      iframe[iL][iF]= v;
    }

  QMetaObject::invokeMethod(
    this, [this]() { display_frame(); }, Qt::QueuedConnection);
}

void FrameViewer::client_status(amjCom::Client, amjCom::Status s) {
  std::cout << "Frameviewer: status: " << std::endl;
  std::cout << "  state=" << s.statedescription() << std::endl;
  std::cout << "  error=" << s.errormessage() << std::endl;
}

void FrameViewer::send_timer_timeout()
{
    if (ui->radioButton_none->isChecked())
        return;

    if (ui->radioButton_sim->isChecked()) {
        testimage(frame);
        iframe= frame;
        display_frame();
        return;
    }

    if (!client)
        return;

    char c;
    if (ui->radioButton_bias->isChecked())
        c = 'B';
    else if (ui->radioButton_diff->isChecked())
        c = 'D';
    else if (ui->radioButton_raw->isChecked())
        c = 'R';
    else if (ui->radioButton_test->isChecked())
        c = 'T';
    else
      return;

    amjCom::Packet p;
    p.write(1)[0] = c;
    client->send(p);
}

void FrameViewer::fps_timer_timeout() {
  ui->label_fps->setText(QString::number(fps_counter));
  fps_counter= 0;
}

void FrameViewer::sliderMin_changed(int v) {
  ui->label_sliderMin->setText(QString::number(v));
}

void FrameViewer::sliderMax_changed(int v) {
  ui->label_sliderMax->setText(QString::number(v));
}

void FrameViewer::display_frame() {
    std::lock_guard<std::mutex> lock(frame_mutex);
    if (ui->checkBox_auto->isChecked()) {
        struct histogram histogram;
        framehistogram(iframe, histogram);
        scalemin = histogrampercentile(histogram, 0.1);
        scalemax = histogrampercentile(histogram, 0.9);
    } else {
        scalemin = ui->verticalSlider_min->value();
        scalemax = ui->verticalSlider_max->value();
    }

    QImage image(iframe.wF(), iframe.wL(), QImage::Format_Grayscale8);
    uchar *outLine;
    int v;
    for(int iH= 0; iH < image.height(); iH++) {
      outLine
        = image.scanLine(image.height() - 1
                         - iH);// To flip it to display as in the ESO software
      for(int iW= 0; iW < image.width(); iW++) {
        v= iframe[iH][iW];
        if(v <= scalemin)
          outLine[iW]= 0;
        else if(v >= scalemax)
          outLine[iW]= 255;
        else
          outLine[iW]
            = static_cast<uchar>((v - scalemin) * 256 / (scalemax - scalemin));
      }
    }

  ui->image->setImage(image);
  fps_counter++;

  // Set information line above frame
  QString info
    = QString("%1/%2/%3 %4:%5:%6 (nL,nF,L0,F0,wL,wF)=(%7,%8,%9,%10,%11,%12)")
        .arg(iframe.time().yr())
        .arg(iframe.time().mo())
        .arg(iframe.time().dy())
        .arg(iframe.time().hr(), 2)
        .arg(iframe.time().mn(), 2)
        .arg(iframe.time().se(), 2)
        .arg(iframe.nL())
        .arg(iframe.nF())
        .arg(iframe.L0())
        .arg(iframe.F0())
        .arg(iframe.wL())
        .arg(iframe.wF());
  ui->label_frameinfo->setText(info);
}

void FrameViewer::framehistogram(const amjFourier::Frame<int> &frame,
                                 struct histogram &histogram) {
  histogram.h.resize(200001);
  for(unsigned int iL= 0; iL < frame.wL(); iL++)
    for(unsigned int iF= 0; iF < frame.wF(); iF++)
      histogram.h[frame[iL][iF] + 100000]++;
  histogram.s= 0;
  for(uint i= 0; i < histogram.h.size(); i++)
    histogram.s+= histogram.h[i];
}

int FrameViewer::histogrampercentile(const struct histogram &histogram,
                                     double percentile) {
  uint64_t threshold= percentile * histogram.s, sum= 0;
  uint value;
  for(value= 0; value < histogram.h.size(); value++) {
    sum+= histogram.h[value];
    if(sum >= threshold)
      break;
  }
  return value - 100000;
}

void FrameViewer::testimage(amjFourier::Frame<float> &frame) {
  int width= 300, height= 200;
  amjTime T;
  T.now();
  frame.resize(height, width);
  frame.time().now();
  for(int iL= 0; iL < height; iL++)
    for(int iF= 0; iF < width; iF++)
      frame[iL][iF]= (iF + iL + seed) % 256;
  seed++;
}
