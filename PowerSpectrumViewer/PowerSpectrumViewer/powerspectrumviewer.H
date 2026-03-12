#ifndef POWERSPECTRUMVIEWER_H
#define POWERSPECTRUMVIEWER_H

#include <QMainWindow>
#include <qcustomplot.h>

#include <amjCom/amjCom.H>
#include <amjCom/amjComTCP.H>
#include <amjCom/amjPacket.H>

#include <amjInterferometry.H>

// #include <QUdpSocket>

// #include "../../shared/include/PowerSpectrum.H"

QT_BEGIN_NAMESPACE
namespace Ui { class PowerSpectrumViewer; }
QT_END_NAMESPACE

class PowerSpectrumViewer : public QMainWindow{
  Q_OBJECT

public:
  PowerSpectrumViewer(QWidget *parent = nullptr);
  ~PowerSpectrumViewer();

private:
  Ui::PowerSpectrumViewer *ui;
  int port;
  // amjPacket packet;
  // QUdpSocket *socket;
  QTimer request_timer;// How often to request a new power spectrum
  QTimer rate_timer;   // 1 s timer for computing actual rate
  int rate_counter;
  amjCom::Client client;
  std::vector<std::string> _names;
  std::vector<QColor> _colors;
  std::vector<amjInterferometry::PowerSpectrum<float>> ps;

  QLabel *timeLabel;
  QLabel *rateLabel;
  QLabel *connectionLabel;

  void server_updated();
  void request_timer_update();
  void rate_timer_callback();
  void client_request();
  void client_receive(amjCom::Client, amjCom::Packet &);
  void client_status(amjCom::Client, amjCom::Status);

  void receive();
  void update_display();
signals:
  void signal_update_display(
    std::vector<std::string>,
    std::vector<amjInterferometry::PowerSpectrum<float>>);
};
#endif // POWERSPECTRUMVIEWER_H
