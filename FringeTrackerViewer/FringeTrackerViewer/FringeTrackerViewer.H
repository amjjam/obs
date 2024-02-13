#ifndef FRINGETRACKERVIEWER_H
#define FRINGETRACKERVIEWER_H

#include <QMainWindow>
#include <QUdpSocket>

#include <amjCom/amjCom.H>

QT_BEGIN_NAMESPACE
namespace Ui { class FringeTrackerViewer; }
QT_END_NAMESPACE

class FringeTrackerViewer : public QMainWindow
{
  Q_OBJECT

public:
  FringeTrackerViewer(QWidget *parent = nullptr);
  ~FringeTrackerViewer();

private:
  Ui::FringeTrackerViewer *ui;
  int port;
  int nBaselines;
  int nStates;
  amjPacket packet;
  QUdpSocket *socket;
  std::vector<std::string> _baselinenames;
  std::vector<std::string> _statenames;
  void receive();
};
#endif // FRINGETRACKERVIEWER_H
