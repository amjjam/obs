#include "Obs.H"
#include "ui_Obs.h"

#include <QtGlobal>
#include <QDateTime>

#include <iostream>

#include "config.H"

Obs::Obs(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::Obs)
{
  ui->setupUi(this);

  processes.append(process({"DataProcessor",ui->label_DataProcessor,ui->checkBox_DataProcessor,
                            ui->amjLED_DataProcessor,"DataProcessor",
                            {"--receiver-frames","/frames:2:10000","--sender-tracker","127.0.0.1:27001",
                             "--sender-phasorviewer","127.0.0.1:27002","$DATAPROCESSOR"},
                            new QProcess}));
  processes.append(process({"FringeTracker",ui->label_FringeTracker,ui->checkBox_FringeTracker,
                            ui->amjLED_FringeTracker,"FringeTracker",
                            {"--active","1",
                             "--receiver-phasors","127.0.0.1:27001",
                             "--sender-pspec","127.0.0.1:27006","100",
                             "--sender-movements","127.0.0.1:27004",
                             "--sender-tracker-stats","127.0.0.1:27008","10","1000",
                             "--sender-tracker-controller","127.0.0.1:27009","1000",
                             "--receiver-tracker-controller","127.0.0.1:27010",
                             "--sender-tracker-snr","127.0.0.1:27011","$FRINGETRACKER"},
                            new QProcess}));
  processes.append(process({"DelayController",ui->label_DelayController,ui->checkBox_DelayController,
                            ui->amjLED_DelayController,"DelayController",
                            {"--receiver-movements","127.0.0.1:27004",
                             "--sender-display","127.0.0.1:27007","100",
                             "--sender-delaylines","sim","6","127.0.0.1:27003"},
                            new QProcess}));
  processes.append(process({"TrackerController",ui->label_TrackerController,ui->checkBox_TrackerController,
                            ui->amjLED_TrackerController,"TrackerController",
                            {},
                            new QProcess}));
  processes.append(process({"FringeTracerViewer",ui->label_FringeTrackerViewer,ui->checkBox_FringeTrackerViewer,
                            ui->amjLED_FringeTrackerViewer,"FringeTrackerViewer",
                            {},
                            new QProcess}));
  processes.append(process({"FrameViewer",ui->label_FrameViewer,ui->checkBox_FrameViewer,
                            ui->amjLED_FrameViewer,"FrameViewer",
                            {},
                            new QProcess}));
  processes.append(process({"DelaylineViewer",ui->label_DelayLineViewer,ui->checkBox_DelayLineViewer,
                            ui->amjLED_DelayLineViewer,"DelaylineViewer",
                            {"--receiver-delaylines-delays","127.0.0.1:27007","$DELAYLINEVIEWER"},
                            new QProcess}));
  processes.append(process({"PowerSpectrumViewer",ui->label_PowerSpectrumViewer,ui->checkBox_PowerSpectrumViewer,
                            ui->amjLED_PowerSpectrumViewer,"PowerSpectrumViewer",
                            {"--receiver-psec","127.0.0.1:27006"},
                            new QProcess}));
  processes.append(process({"PhasorViewer",ui->label_PhasorViewer,ui->checkBox_PhasorViewer,
                            ui->amjLED_PhasorViewer,"PhasorViewer",
                            {"--receiver-phasors","127.0.0.1:27002","$PHASORVIEWER"},
                            new QProcess}));
  processes.append(process({"Simulator",ui->label_Simulator,ui->checkBox_Simulator,
                            ui->amjLED_Simulator,"Simulator",
                            {"--sender-frames","/frames:2:10000",
                             "--sender-frames2","/frameviewer:2:10000","100",
                             "--receiver-delaylines","127.0.0.1:27003","$SIMULATOR"},
                            new QProcess}));
  processes.append(process({"SNRViewer",ui->label_SNRViewer,ui->checkBox_SNRViewer,
                           ui->amjLED_SNRViewer,"SNRViewer",
                           {"--receiver-tracker-snr","127.0.0.1:27011","$SNRVIEWER"},
                           new QProcess}));

  connect(ui->toolButton_ConfigFile,&QToolButton::clicked,this,&Obs::load_config);
  connect(ui->toolButton_LogFile,&QToolButton::clicked,this,&Obs::set_logfile);

  QVector<QColor> colors({QColor(Qt::black),QColor(Qt::red),QColor(Qt::green),
                          QColor(Qt::blue),QColor(Qt::yellow)});
  for(int i=0;i<processes.size();i++){
    processes[i].led->setColors(colors);
    processes[i].process->setProgram(processes[i].program);
    processes[i].process->setArguments(processes[i].arguments);
    connect(processes[i].checkBox,&QCheckBox::stateChanged,this,&Obs::checkbox);
    connect(processes[i].process,&QProcess::stateChanged,this,&Obs::processstate);
    connect(processes[i].process,&QProcess::errorOccurred,this,&Obs::processerror);
    connect(processes[i].process,&QProcess::readyReadStandardError,this,&Obs::processstderr);
    connect(processes[i].process,&QProcess::readyReadStandardOutput,this,&Obs::processstdout);
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
    QStringList arguments;
    if(format_arguments(processes[i].arguments,config,arguments)){
      processes[i].process->setArguments(arguments);
      processes[i].process->start();
    }
    else
      processes[i].checkBox->setCheckState(Qt::Unchecked);
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

void Obs::processstderr(){
  int i;
  for(i=0;i<processes.size();i++)
    if(sender()==processes[i].process)
      break;
  if(i==processes.size()){
    std::cerr << "Unknown object sent signal to Obs::processstederr" << std::endl;
    abort();
  }
  QString msg=QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss")+" "+processes[i].name+": ";
  char buf[1024];
  int n;
  n=processes[i].process->readLine(buf,1024);
  if(n>0)
    msg+=buf;
  while(msg.endsWith(' ' ) || msg.endsWith( '\n' )) msg.chop(1);
  if(ui->checkBox_stderr->isChecked())
    ui->textEdit_log->append(msg);
  if(logstream.is_open())
    logstream << msg.toStdString() << std::endl;
}

void Obs::processstdout(){
  int i;
  for(i=0;i<processes.size();i++)
    if(sender()==processes[i].process)
      break;
  if(i==processes.size()){
    std::cerr << "Unknown object sent signal to Obs::processstederr" << std::endl;
    abort();
  }
  QString msg=QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm:ss")+" "+processes[i].name+": ";
  char buf[1024];
  int n;
  n=processes[i].process->readLine(buf,1024);
  if(n>0)
    msg+=buf;
  while(msg.endsWith(' ' ) || msg.endsWith( '\n' )) msg.chop(1);
  if(ui->checkBox_stdout->isChecked())
    ui->textEdit_log->append(msg);
  if(logstream.is_open())
    logstream << msg.toStdString() << std::endl;
}

void Obs::load_config(){
  std::cout << "Loading config file" << std::endl;
  QString filename;
  filename=QFileDialog::getOpenFileName(this,"Configuration file",".","");
  if(filename.size()>0){
    config=Config(filename.toStdString());
    config.print();
    ui->lineEdit_ConfigFile->setText(filename);
  }
  else
    ui->lineEdit_ConfigFile->setText(QString(""));
}

void Obs::set_logfile(){
  logfilename=QFileDialog::getSaveFileName(this,"Log file",".","");
  if(logfilename.size()>0){
    if(logstream.is_open())
      logstream.close();
    logstream.open(logfilename.toStdString().c_str());
    ui->lineEdit_LogFile->setText(logfilename);
  }
}



bool Obs::format_arguments(const QStringList ain, const Config &c, QStringList &aout){
  aout=ain;
  for(int i=0;i<aout.size();i++){
    QString s=aout[i];
    if(aout[i].front()=='$'){
      s.remove(0,1);
      std::vector<std::string> ss;
      if(c.find(s.toStdString(),ss)){
        aout.removeAt(i);
        for(unsigned int j=0;j<ss.size();j++)
          aout.append(QString::fromStdString(ss[j]));
      }
      else{
         std::cerr << "Could not find " << s.toStdString() << " in configuration" << std::endl;
         return false;
      }
    }
  }
  // int j;
  // for(int i=0;i<aout.size();i++){
  //   QString s=aout[i];
  //   if((j=s.indexOf('@'))>=0){
  //     if(
  //   }
  // }
  return true;
}
