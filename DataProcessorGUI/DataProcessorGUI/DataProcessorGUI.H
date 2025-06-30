#ifndef DATAPROCESSORGUI_H
#define DATAPROCESSORGUI_H

#include <QMainWindow>
#include <QTimer>

#include <amjCom/amjComTCP.H>

#include "../../DataProcessor/include/DataProcessorStatus.H"

QT_BEGIN_NAMESPACE
namespace Ui {
  class DataProcessorGUI;
}
QT_END_NAMESPACE

class DataProcessorGUI: public QMainWindow {
  Q_OBJECT

public:
  DataProcessorGUI(QWidget *parent= nullptr);
  ~DataProcessorGUI();

private:
  Ui::DataProcessorGUI *ui;
  QTimer update_timer;
  amjCom::Client client;
  void server_updated();
  void button_clicked();
  void update_timer_timeout();
  void client_receive(amjCom::Client, amjCom::Packet &);
  void client_status(amjCom::Client, amjCom::Status);
};
#endif// DATAPROCESSORGUI_H
