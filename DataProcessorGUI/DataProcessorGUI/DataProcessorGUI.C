#include "DataProcessorGUI.H"
#include "ui_DataProcessorGUI.h"

#include <QFileDialog>

#include <amjQCom.H>

#include <amjCom/amjComTCP.H>

DataProcessorGUI::DataProcessorGUI(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DataProcessorGUI) {
  ui->setupUi(this);
  clientStatus= new amjQCom::ClientStatus(this);
  ui->statusBar->addPermanentWidget(clientStatus);
  connect(ui->lineEdit_server, &QLineEdit::editingFinished, this,
          &DataProcessorGUI::server_updated);
  connect(ui->pushButton_stop, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  connect(ui->pushButton_phasors, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  connect(ui->pushButton_bias, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  connect(ui->pushButton_FileStartStop, &QPushButton::clicked, this,
          &DataProcessorGUI::button_clicked);
  connect(ui->toolButton_File, &QToolButton::clicked, this, [this]() {
    ui->lineEdit_FileOutput->setText(QFileDialog::getSaveFileName());
  });
  update_timer.callOnTimeout(this, &DataProcessorGUI::update_timer_timeout);
  update_timer.start(1000);
  // server_updated();// Connect with address already in field
}

DataProcessorGUI::~DataProcessorGUI() { delete ui; }

void DataProcessorGUI::server_updated() {
  std::cout << "client.use_count()=" << client.use_count() << std::endl;
  client.reset();
  client= amjCom::TCP::create_client(
    ui->lineEdit_server->text().toStdString(),
    [this](amjCom::Client c, amjCom::Packet &p) { client_receive(c, p); },
    [this](amjCom::Client c, amjCom::Status s) {
      clientStatus->pushStatus(s);
      /*client_status(c, s);*/
    });
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
  else if(sender() == ui->pushButton_FileStartStop)
    c= 'F';

  amjCom::Packet p;
  p.write(1)[0]= c;
  if(sender() == ui->pushButton_bias) {
    uint32_t nBias= ui->lineEdit_nframes->text().toUInt();
    memcpy(p.write(sizeof(uint32_t)), &nBias, sizeof(uint32_t));
  } else if(sender() == ui->pushButton_FileStartStop) {
    if(ui->pushButton_FileStartStop->text() == "Start") {
      std::cout << "Start" << std::endl;
      p.write(1)[0]= 'O';
      if(ui->radioButton_FileCounts->isChecked())
        p.write(1)[0]= 'C';
      else
        p.write(1)[0]= 'F';
      uint32_t l= ui->lineEdit_FileOutput->text().length();
      memcpy(p.write(sizeof(uint32_t)), &l, sizeof(uint32_t));
      memcpy(p.write(l), ui->lineEdit_FileOutput->text().toStdString().c_str(),
             l);
      ui->pushButton_FileStartStop->setText("Stop");
    } else {
      std::cout << "Stop" << std::endl;
      p.write(1)[0]= 'C';
      ui->pushButton_FileStartStop->setText("Start");
    }
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
  ui->label_fps->setText(QString::number(status.fps, 'f', 1));
  if(!status.hasBias) {
    ui->label_t_bias->setText("Never");
    ui->label_n_bias->setText("0");
  } else {
    ui->label_t_bias->setText(QString("%1/%2/%3 %4:%5:%6")
                                .arg(status.biastime.yr())
                                .arg(status.biastime.mo())
                                .arg(status.biastime.dy())
                                .arg(status.biastime.hr(), 2, 10, QChar('0'))
                                .arg(status.biastime.mn(), 2, 10, QChar('0'))
                                .arg(status.biastime.se(), 2, 10, QChar('0')));
    ui->label_n_bias->setText(QString::number(status.nBias));
  }
  std::cout << "fileframes=" << status.fileframes << std::endl;
  if(status.fileframes < 0)
    ui->label_FileFrames->setText("write error");
  else
    ui->label_FileFrames->setText(QString::number(status.fileframes));
}

void DataProcessorGUI::client_status(amjCom::Client, amjCom::Status s) {
  ui->label_connection_state->setText(
    QString::fromStdString(s.statedescription()));
}
