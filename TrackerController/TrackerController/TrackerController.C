#include "TrackerController.H"
#include "ui_TrackerController.h"

#include <QString>
#include <QNetworkDatagram>

#include <cmath>
#include <cstring>

#include "../../FringeTracker/include/FringeTrackerStateMachine.H"

#define FTSM_LOCAL_PORT 27009
#define FTSM_DESTINATION_PORT 27010

#define LED_RED 0
#define LED_YELLOW 1
#define LED_GREEN 2

TrackerController::TrackerController(QWidget *parent): QMainWindow(parent), ui(new Ui::TrackerController){
  ui->setupUi(this);
  ui->widget_LED->setColors({Qt::red,Qt::yellow,Qt::green});
  ui->widget_LED->startWatchdog(5000);
  baselineButtons={ui->radioButton1,ui->radioButton2,ui->radioButton3,
                   ui->radioButton4,ui->radioButton5,ui->radioButton6};
  baselineCheckBoxes={ui->checkBox_baseline1,ui->checkBox_baseline2,ui->checkBox_baseline3,
                      ui->checkBox_baseline4,ui->checkBox_baseline5,ui->checkBox_baseline6};
  baselineFoundSNRs={ui->lineEdit_SNRf1,ui->lineEdit_SNRf2,ui->lineEdit_SNRf3,
                     ui->lineEdit_SNRf4,ui->lineEdit_SNRf5,ui->lineEdit_SNRf6};
  baselineLostSNRs={ui->lineEdit_SNRl1,ui->lineEdit_SNRl2,ui->lineEdit_SNRl3,
                    ui->lineEdit_SNRl4,ui->lineEdit_SNRl5,ui->lineEdit_SNRl6};
  ftsm=new QUdpSocket(this);
  ftsm->bind(QHostAddress::LocalHost,FTSM_LOCAL_PORT);
  connect(ftsm,&QUdpSocket::readyRead,this,&TrackerController::ftsm_receive);
}

#include <iostream>

TrackerController::~TrackerController(){
  delete ui;
}

void TrackerController::slider(int v){
  float snr=exp((float)(v-SNR_MIN)/(float)(SLIDER_MAX-SLIDER_MIN)*(log(SNR_MAX)-log(SNR_MIN))+log(SNR_MIN));
  for(int i=0;i<baselineFoundSNRs.size();i++)
    if(baselineButtons[i]->isChecked()){
      if(sender()==ui->verticalSlider_found)
        baselineFoundSNRs[i]->setText(QString::number(snr));
      else
        baselineLostSNRs[i]->setText(QString::number(snr));
    }
}

void TrackerController::ftsm_ui(){
  std::cout << "ftsm_ui()" << std::endl;
  ftsm_write();
  ftsm_send();
}

void TrackerController::ftsm_write(){
  std::cout << "ftsm_write()" << std::endl;
  Qt::CheckState checkstate;
  smcs.resize(names.size());
  for(unsigned int i=0;i<smcs.size();i++){
    checkstate=baselineCheckBoxes[i]->checkState();
    if(checkstate==Qt::Unchecked)
      smcs[i].onoff=0;
    else
      smcs[i].onoff=1;
    smcs[i].snrFound=baselineFoundSNRs[i]->text().toFloat();
    smcs[i].snrLost=baselineLostSNRs[i]->text().toFloat();
    smcs[i].holdSearch=ui->lineEdit_search->text().toInt();
    smcs[i].holdFound=ui->lineEdit_found->text().toInt();
    smcs[i].holdLost=ui->lineEdit_lost->text().toInt();
    smcs[i].searchAMax=ui->lineEdit_Amax->text().toFloat();
    smcs[i].searchAMin=ui->lineEdit_Amin->text().toFloat();
    smcs[i].searchStepSize=ui->lineEdit_step->text().toFloat();
    smcs[i].searchFactor=ui->lineEdit_factor->text().toFloat();
    smcs[i].maxGD=ui->lineEdit_max->text().toFloat();
    smcs[i].gain=ui->lineEdit_gain->text().toFloat();
  }
}

void TrackerController::ftsm_send(){
  std::cout << "ftsm_send()" << std::endl;
  QByteArray data;
  QNetworkDatagram datagram;
  amjPacket packet;
  std::cout << names.size() << std::endl;
  packet << (int)names.size();
  for(unsigned int i=0;i<names.size();i++)
    smcs[i].write(packet.write(smcs[i].size()));
  std::cout << "packet.size()=" << packet.size() << std::endl;
  data.resize(packet.size());
  memcpy(data.data(),packet._data(),packet.size());
  datagram.setData(data);
  datagram.setDestination(address,FTSM_DESTINATION_PORT);
  ftsm->writeDatagram(datagram);
  std::cout << datagram.data()[0] << " " << datagram.data()[1] << " " << datagram.data()[2] << " " << datagram.data()[3] << std::endl;
}

void TrackerController::ftsm_receive(){
  std::cout << "ftsm_receive()" << std::endl;
  QNetworkDatagram datagram;
  amjPacket packet;
  std::vector<std::string> names;
  std::vector<FringeTrackerStateMachineConfig> smcs;
  while(ftsm->hasPendingDatagrams()){
    datagram=ftsm->receiveDatagram();
    packet.clear();
    std::cout << "datagram: " << datagram.data().size() << std::endl;
    memcpy(packet.write(datagram.data().size()),datagram.data().data(),datagram.data().size());
    packet.reset();
    packet >> names;
    smcs.resize(names.size());
    for(unsigned int i=0;i<names.size();i++){
      smcs[i].read(packet.read(smcs[i].size()));
      //smcs[i].print();
    }
  }
  updateNames(names);

  if(smcs!=TrackerController::smcs){
      ui->widget_LED->set(LED_YELLOW);
      ui->widget_LED->resetWatchdog();
      std::cout << "TrackerController::smcs.size()=" << TrackerController::smcs.size() << ", smcs.size()=" << smcs.size() << std::endl;
      for(int i=0;i<TrackerController::smcs.size();i++){
        std::cout << "TrackerController::smcs[" << i << "]" << std::endl;
        TrackerController::smcs[i].print();
      }
      for(int i=0;i<smcs.size();i++){
        std::cout << "smcs[" << i << "]" << std::endl;
        smcs[i].print();
      }
  }
  else{
      ui->widget_LED->set(LED_GREEN);
      ui->widget_LED->resetWatchdog();
  }
}

void TrackerController::updateNames(std::vector<std::string> _names){
  if(_names==names)
    return;
  names=_names;
  for(unsigned int i=0;i<names.size();i++){
    baselineButtons[i]->setText(QString::fromStdString(names[i]));
    baselineButtons[i]->setEnabled(true);
    baselineCheckBoxes[i]->setEnabled(true);
    baselineFoundSNRs[i]->setEnabled(true);
    baselineLostSNRs[i]->setEnabled(true);
  }
  for(unsigned int i=names.size();i<(unsigned int)baselineButtons.size();i++){
    baselineButtons[i]->setText("");
    baselineButtons[i]->setDisabled(true);
    baselineCheckBoxes[i]->setDisabled(true);
    baselineCheckBoxes[i]->setCheckState(Qt::CheckState::Unchecked);
    baselineFoundSNRs[i]->setDisabled(true);
    baselineLostSNRs[i]->setDisabled(true);
  }
}
