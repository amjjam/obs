#ifndef FRAMEVIEWER_H
#define FRAMEVIEWER_H

#include <QMainWindow>
#include <QThread>

#include <amjCom/amjPacket.H>

QT_BEGIN_NAMESPACE
namespace Ui { class FrameViewer; }
QT_END_NAMESPACE

class Receiver:public QThread{
  Q_OBJECT
public:
  void run() override;
signals:
  void received(amjPacket);
};

class FrameViewer : public QMainWindow
{
  Q_OBJECT

public:
  FrameViewer(QWidget *parent = nullptr);
  ~FrameViewer();
  void receive(amjPacket);
private:
  Ui::FrameViewer *ui;
  Receiver *receiver;
};


#endif // FRAMEVIEWER_H
