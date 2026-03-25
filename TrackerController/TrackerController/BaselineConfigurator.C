#include "BaselineConfigurator.H"
#include "ui_BaselineConfigurator.h"

BaselineConfigurator::BaselineConfigurator(QWidget *parent)
  : QDialog(parent), ui(new Ui::BaselineConfigurator) {
  ui->setupUi(this);
}

BaselineConfigurator::~BaselineConfigurator() { delete ui; }
