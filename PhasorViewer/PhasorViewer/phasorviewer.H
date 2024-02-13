#ifndef PHASORVIEWER_H
#define PHASORVIEWER_H

#include <QMainWindow>
#include <amjCom/amjPacket.H>

#include <QUdpSocket>
#include "../../shared/include/Phasors.H"
#include "../../shared/include/Wavelengths.H"
#include <qcustomplot.h>

QT_BEGIN_NAMESPACE
namespace Ui { class PhasorViewer; }
QT_END_NAMESPACE

class PhasorViewer : public QMainWindow
{
  Q_OBJECT

public:
  PhasorViewer(QWidget *parent = nullptr);
  ~PhasorViewer();

private:
  Ui::PhasorViewer *ui;
  int port;
  amjPacket packet;
  QUdpSocket *socket;
  QCPAxisRect *v2plot;
  QCPAxisRect *phiplot;
  void receive();
  void update_plots(PhasorSets &p);
};
#endif // PHASORVIEWER_H
