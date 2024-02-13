#ifndef POWERSPECTRUMVIEWER_H
#define POWERSPECTRUMVIEWER_H

#include <QMainWindow>
#include <qcustomplot.h>

#include <amjCom/amjPacket.H>

#include <QUdpSocket>

#include "../../shared/include/PowerSpectrum.H"

QT_BEGIN_NAMESPACE
namespace Ui { class PowerSpectrumViewer; }
QT_END_NAMESPACE

class PowerSpectrumViewer : public QMainWindow
{
  Q_OBJECT

public:
  PowerSpectrumViewer(QWidget *parent = nullptr);
  ~PowerSpectrumViewer();

private:
  Ui::PowerSpectrumViewer *ui;
  int port;
  amjPacket packet;
  QUdpSocket *socket;
  std::vector<std::string> _names;
  std::vector<QColor> _colors;
  void receive();

};
#endif // POWERSPECTRUMVIEWER_H
