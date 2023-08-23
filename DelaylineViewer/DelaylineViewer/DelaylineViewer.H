#ifndef DELAYLINEVIEWER_H
#define DELAYLINEVIEWER_H

#include <QMainWindow>
#include <QUdpSocket>

#include <amjChart.H>

#include "../../shared/include/Delays.H"

QT_BEGIN_NAMESPACE
namespace Ui { class DelaylineViewer; }
QT_END_NAMESPACE

class DelaylineViewer : public QMainWindow
{
  Q_OBJECT

public:
  DelaylineViewer(QWidget *parent = nullptr);
  ~DelaylineViewer();

private:
  Ui::DelaylineViewer *ui;
  int port;
  amjPacket packet;
  QUdpSocket *socket;
  std::vector<std::string> delaylinenames;
  void receive();
};
#endif // DELAYLINEVIEWER_H
