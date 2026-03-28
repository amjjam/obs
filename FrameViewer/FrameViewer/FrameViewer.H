#ifndef FRAMEVIEWER_H
#define FRAMEVIEWER_H

#include <QLabel>
#include <QMainWindow>
#include <QRadioButton>
#include <QThread>
#include <QTimer>

#include <amjQCom.H>

#include <amjCom/amjCom.H>
#include <amjFourier.H>
#include <amjTime.H>

#include <amjWidgets.H>

QT_BEGIN_NAMESPACE
namespace Ui { class FrameViewer; }
QT_END_NAMESPACE

class ScaledImageWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScaledImageWidget(QWidget *parent = nullptr);
    void setImage(const QImage &image);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
  QImage image;
};


class FrameViewer : public QMainWindow
{
  Q_OBJECT

public:
  FrameViewer(QWidget *parent = nullptr);
  ~FrameViewer();
  void receive(amjPacket);
signals:
  void signal_display_frame();

private:
  Ui::FrameViewer *ui;
  amjWidgets::TimeDisplay *timeDisplay;
  amjWidgets::RateCounter *rateCounter;
  amjQCom::ClientStatus *clientStatus;
  int seed;
  int range;
  double scalemin,scalemax;

  amjFourier::Frame<float> frame;
  amjFourier::Frame<int> iframe;
  amjTime time;
  bool request_in_flight;
  amjCom::Client client;
  QTimer timer;
  char frame_type;
  mutable std::mutex frame_mutex;
  mutable std::mutex state_mutex;
  
  void radio_none_or_sim_toggled(bool);
  void auto_clicked(Qt::CheckState);
  void range_updated();
  void server_updated();

  void client_receive(amjCom::Client, amjCom::Packet &);
  void client_status(amjCom::Client, amjCom::Status);

  void update_scale();
  void update_timer();

  void mode_changed(bool);
  void timer_timeout();
  void fps_timer_timeout();

  void sliderMin_changed(int);
  void sliderMax_changed(int);

  void clear_display();
  void display_frame();
  struct Histogram{
    uint64_t sum=0;
    std::vector<uint32_t> bins;
  };
  void framehistogram(const amjFourier::Frame<int> &, struct Histogram &h);
  int histogrampercentile(const struct Histogram &, double);

  void testimage(amjFourier::Frame<float> &);
};


#endif // FRAMEVIEWER_H
