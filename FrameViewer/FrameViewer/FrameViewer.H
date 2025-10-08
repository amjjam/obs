#ifndef FRAMEVIEWER_H
#define FRAMEVIEWER_H

#include <QLabel>
#include <QMainWindow>
#include <QRadioButton>
#include <QThread>
#include <QTimer>

#include <amjCom/amjCom.H>
#include <amjFourier.H>
#include <amjTime.H>

QT_BEGIN_NAMESPACE
namespace Ui { class FrameViewer; }
QT_END_NAMESPACE

class Receiver:public QThread{
  Q_OBJECT
public:
signals:
  void received(amjPacket);
};

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
protected:
           //void resizeEvent(QResizeEvent *event) override;
private:
  Ui::FrameViewer *ui;
  int seed;
  int range;
  double scalemin,scalemax;
  int fps_counter;

  Frame<float> frame;
  Frame<int> iframe;
  amjTime time;
  amjCom::Client client;
  QTimer send_timer;
  QTimer fps_timer;
  void radio_none_or_sim_toggled(bool);
  void auto_clicked(Qt::CheckState);
  void range_updated();
  void server_updated();

  void client_receive(amjCom::Client, amjCom::Packet &);
  void client_status(amjCom::Client, amjCom::Status);

  void update_scale();
  void update_timer();

  void send_timer_timeout();
  void fps_timer_timeout();

  void sliderMin_changed(int);
  void sliderMax_changed(int);

  void display_frame();
  struct histogram{
    uint64_t s;
    std::vector<uint32_t> h;
  };
  void framehistogram(const Frame<int> &, struct histogram &h);
  int histogrampercentile(const struct histogram &, double);

  void testimage(Frame<float> &);
};


#endif // FRAMEVIEWER_H
