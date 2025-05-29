#ifndef PROCESSORCONTROLLER_H
#define PROCESSORCONTROLLER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ProcessorController; }
QT_END_NAMESPACE

class ProcessorController : public QMainWindow
{
  Q_OBJECT

public:
  ProcessorController(QWidget *parent = nullptr);
  ~ProcessorController();

private:
  Ui::ProcessorController *ui;
};
#endif // PROCESSORCONTROLLER_H
