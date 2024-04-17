#ifndef OBS_H
#define OBS_H

#include <iostream>
#include <fstream>

#include <QMainWindow>
#include <QPushButton>
#include <QProcess>
#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>

#include <amjWidgets.H>

#include "config.H"

QT_BEGIN_NAMESPACE
namespace Ui { class Obs; }
QT_END_NAMESPACE

class Obs : public QMainWindow
{
  Q_OBJECT

public:
  Obs(QWidget *parent = nullptr);
  ~Obs();

private:
  Ui::Obs *ui;
  struct process{
    QString name;
    QLabel *label;
    QCheckBox *checkBox;
    amjLED *led;
    QString program;
    QStringList arguments;
    QProcess *process;
  };
  QVector<struct process> processes;
  Config config;
  QString logfilename;
  std::ofstream logstream;
  void checkbox(int);
  void processstate(QProcess::ProcessState);
  void processerror(QProcess::ProcessError);
  void processstderr();
  void processstdout();
  void load_config();
  void set_logfile();
  bool format_arguments(const QStringList, const Config &,QStringList &);
  //  void finished(int,QProcess::ExitStatus);
};
#endif // OBS_H
