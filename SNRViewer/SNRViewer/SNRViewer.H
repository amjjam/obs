#ifndef SNRVIEWER_H
#define SNRVIEWER_H

#include <QMainWindow>
#include <QUdpSocket>

#include <amjChart.H>
#include <amjCom/amjPacket.H>

QT_BEGIN_NAMESPACE
namespace Ui { class SNRViewer; }
QT_END_NAMESPACE

class SNRViewer : public QMainWindow
{
  Q_OBJECT

public:
  SNRViewer(QWidget *parent = nullptr);
  ~SNRViewer();

private:
  Ui::SNRViewer *ui;
  int port;
  amjPacket packet;
  QUdpSocket *socket;
  std::vector<std::string> baselinenames;
  void receive();
};
#endif // SNRVIEWER_H
