#ifndef BASELINECONFIGURATOR_H
#define BASELINECONFIGURATOR_H

#include <QDialog>

namespace Ui {
  class BaselineConfigurator;
}

class BaselineConfigurator: public QDialog {
  Q_OBJECT

public:
  explicit BaselineConfigurator(QWidget *parent= nullptr);
  ~BaselineConfigurator();

private:
  Ui::BaselineConfigurator *ui;
};

#endif// BASELINECONFIGURATOR_H
