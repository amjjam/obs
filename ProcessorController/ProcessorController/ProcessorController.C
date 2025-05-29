amj#include "ProcessorController.H"
#include "ui_ProcessorController.h"

ProcessorController::ProcessorController(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::ProcessorController)
{
  ui->setupUi(this);
}

ProcessorController::~ProcessorController()
{
  delete ui;
}

