#ifndef OBS_H
#define OBS_H

#include <QMainWindow>
#include <QPushButton>
#include <QProcess>
#include <QCheckBox>
#include <QLabel>

#include <amjWidgets.H>

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
    QLabel *label;
    QCheckBox *checkBox;
    amjLED *led;
    QString program;
    QStringList arguments;
    QProcess *process;
  };
  QVector<struct process> processes;
  void checkbox(int);
  void processstate(QProcess::ProcessState);
  void processerror(QProcess::ProcessError);
  //  void finished(int,QProcess::ExitStatus);
};
#endif // OBS_H
