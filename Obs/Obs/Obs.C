#include "Obs.H"
#include "ui_Obs.h"

#include <QtGlobal>

#include <iostream>

Obs::Obs(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::Obs)
{
  ui->setupUi(this);

  processes.append(process({ui->label_DataProcessor,ui->checkBox_DataProcessor,
                            ui->amjLED_DataProcessor,"DataProcessor",
                            {"--receiver-frames","/frames:2:10000","--sender-tracker","127.0.0.1:27001",
                             "--sender-phasorviewer","127.0.0.1:27002"},
                            new QProcess}));
  processes.append(process({ui->label_FringeTracker,ui->checkBox_FringeTracker,
                            ui->amjLED_FringeTracker,"FringeTracker",
                            {"--sender-pspec","127.0.0.1:27006","100",
                             "--sender-movements","127.0.0.1:27004",
                             "--sender-tracker-stats","127.0.0.1:27008","10","1000",
                             "--sender-tracker-controller","127.0.0.1:27009","1000",
                             "--receiver-tracker-controller","127.0.0.1:27010"},
                            new QProcess}));
  processes.append(process({ui->label_DelayController,ui->checkBox_DelayController,
                            ui->amjLED_DelayController,"DelayController",
                            {"--receiver-movements","127.0.0.1:27004",
                             "--sender-display","127.0.0.1:27007","100",
                             "--sender-delaylines","sim","6","127.0.0.1:27003"},
                            new QProcess}));
  processes.append(process({ui->label_TrackerController,ui->checkBox_TrackerController,
                            ui->amjLED_TrackerController,"TrackerController",
                            {},
                            new QProcess}));
  processes.append(process({ui->label_FringeTrackerViewer,ui->checkBox_FringeTrackerViewer,
                            ui->amjLED_FringeTrackerViewer,"FringeTrackerViewer",
                            {},
                            new QProcess}));
  processes.append(process({ui->label_FrameViewer,ui->checkBox_FrameViewer,
                            ui->amjLED_FrameViewer,"FrameViewer",
                            {},
                            new QProcess}));
  processes.append(process({ui->label_DelayLineViewer,ui->checkBox_DelayLineViewer,
                            ui->amjLED_DelayLineViewer,"DelaylineViewer",
                            {"--receiver-delaylines-delays","127.0.0.1:27007"},
                            new QProcess}));
  processes.append(process({ui->label_PowerSpectrumViewer,ui->checkBox_PowerSpectrumViewer,
                            ui->amjLED_PowerSpectrumViewer,"PowerSpectrumViewer",
                            {"--receiver-psec","127.0.0.1:27006"},
                            new QProcess}));
  processes.append(process({ui->label_PhasorViewer,ui->checkBox_PhasorViewer,
                            ui->amjLED_PhasorViewer,"PhasorViewer",
                            {"--receiver-phasors","127.0.0.1:27002"},
                            new QProcess}));
  processes.append(process({ui->label_Simulator,ui->checkBox_Simulator,
                            ui->amjLED_Simulator,"Simulator",
                            {"--sender-frames","/frames:2:10000",
                             "--sender-frames2","/frameviewer:2:10000","100",
                             "--receiver-delaylines","127.0.0.1:27003"},
                            new QProcess}));

  QVector<QColor> colors({QColor(Qt::black),QColor(Qt::red),QColor(Qt::green),
                          QColor(Qt::blue),QColor(Qt::yellow)});
  for(int i=0;i<processes.size();i++){
    processes[i].led->setColors(colors);
    processes[i].process->setProgram(processes[i].program);
    processes[i].process->setArguments(processes[i].arguments);
    connect(processes[i].checkBox,&QCheckBox::stateChanged,this,&Obs::checkbox);
    connect(processes[i].process,&QProcess::stateChanged,this,&Obs::processstate);
    connect(processes[i].process,&QProcess::errorOccurred,this,&Obs::processerror);
  }
}

Obs::~Obs(){
  for(int i=0;i<processes.size();i++)
    delete processes[i].process;

  delete ui;
}

void Obs::checkbox(int state){
  std::cout << "checkbox: " << state << std::endl;
  int i;
  for(i=0;i<processes.size();i++)
    if(processes[i].checkBox==sender())
      break;
  if(i==processes.size()){
    std::cerr << "error: received signal from unrecognized checkbox" << std::endl;
    abort();
  }
  if(state==Qt::Checked){
    processes[i].process->start();
  }
  else{
    processes[i].process->kill();
  }
}

void Obs::processstate(QProcess::ProcessState state){
  std::cout << "processstate: " << state << std::endl;
  int i;
  for(i=0;i<processes.size();i++)
    if(processes[i].process==sender())
      break;
  if(i==processes.size()){
    std::cerr << "error: received signal from urecognized process" << std::endl;
    abort();
  }
  if(state==QProcess::Running)
    processes[i].led->set(2);
  else if(state==QProcess::Starting)
    processes[i].led->set(4);
  else if(state==QProcess::NotRunning){
    processes[i].led->set(0);
    processes[i].checkBox->setCheckState(Qt::Unchecked);
  }
}

void Obs::processerror(QProcess::ProcessError error){
  std::cerr << "process reported error: " << error << std::endl;
  int i;
  for(i=0;i<processes.size();i++)
    if(processes[i].process==sender())
      break;
  if(i==processes.size()){
    std::cerr << "error: received signal from unrecognized process" << std::endl;
    abort();
  }
  processes[i].led->set(1);
}
