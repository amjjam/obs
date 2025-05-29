#ifndef DELAYCOMMANDER_H
#define DELAYCOMMANDER_H

#include <QMainWindow>

#define CMD_MOVE 0
#define CMD_HOME 1

#define NDELAYLINES 6

QT_BEGIN_NAMESPACE
namespace Ui { class DelayCommander; }
QT_END_NAMESPACE

class DelayCommander : public QMainWindow
{
  Q_OBJECT

public:
  DelayCommander(QWidget *parent = nullptr);
  ~DelayCommander();

private:
  Ui::DelayCommander *ui;
  void MoveClicked();
  void ClearClicked();
  void HomeClicked();
};
#endif // DELAYCOMMANDER_H
