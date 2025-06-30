#include "DataProcessorGUI.H"
#include "ui_DataProcessorGUI.h"

DataProcessorGUI::DataProcessorGUI(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DataProcessorGUI) {
  ui->setupUi(this);
  connect(ui->lineEdit_server, &QLineEdit::editingFinished, this,
          &DataProcessorGUI::server_updated);
  connect(ui->pushButton_stop, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  connect(ui->pushButton_phasors, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  connect(ui->pushButton_bias, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  update_timer.callOnTimeout(this, &DataProcessorGUI::update_timer_timeout);
  update_timer.start(1000);
  server_updated();// Connect with address already in field
}

DataProcessorGUI::~DataProcessorGUI() { delete ui; }

void DataProcessorGUI::server_updated() {
  client.reset();
  client= amjCom::TCP::create_client(
    ui->lineEdit_server->text().toStdString(),
    [this](amjCom::Client c, amjCom::Packet &p) { client_receive(c, p); },
    [this](amjCom::Client c, amjCom::Status s) { client_status(c, s); });
  client->start();
}

#include <stdint.h>
void DataProcessorGUI::button_clicked() {
  if(!client)
    return;
  char c= ' ';
  if(sender() == ui->pushButton_stop)
    c= 'S';
  else if(sender() == ui->pushButton_phasors)
    c= 'P';
  else if(sender() == ui->pushButton_bias)
    c= 'B';

  amjCom::Packet p;
  p.write(1)[0]= c;
  if(sender() == ui->pushButton_bias) {
    uint32_t nBias= ui->lineEdit_nframes->text().toUInt();
    memcpy(p.write(sizeof(uint32_t)), &nBias, sizeof(uint32_t));
  }
  client->send(p);
}

void DataProcessorGUI::update_timer_timeout() {
  std::cout << "update_timer_timeout" << std::endl;
  if(!client)
    return;

  amjCom::Status s= client->status();
  ui->label_connection_state->setText(
    QString::fromStdString(s.statedescription()));
  // Send U to DataProcessor
  std::cout << "Send U to DataProcessor" << std::endl;
  amjCom::Packet p;
  p.write(1)[0]= 'U';
  client->send(p);
}

void DataProcessorGUI::client_receive(amjCom::Client, amjCom::Packet &p) {
  std::cout << "client_receive" << std::endl;
  DataProcessorStatus status;
  status.read(p.read(status.size()));
  if(status.state == 'S')
    ui->label_processor_state->setText(QString::fromUtf8("Stopped"));
  else if(status.state == 'P')
    ui->label_processor_state->setText(QString::fromUtf8("Phasors"));
  else if(status.state == 'B')
    ui->label_processor_state->setText(QString::fromUtf8("Bias"));
}

void DataProcessorGUI::client_status(amjCom::Client, amjCom::Status s) {
  ui->label_connection_state->setText(
    QString::fromStdString(s.statedescription()));
}
