#include "DelayCommander.H"
#include "ui_DelayCommander.h"

#include <amjCom/amjPacket.H>
#include <amjTime.H>

#include "../../shared/include/Delays.H"


DelayCommander::DelayCommander(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::DelayCommander)
{
  ui->setupUi(this);
}

DelayCommander::~DelayCommander()
{
  delete ui;
}

void DelayCommander::MoveClicked(){
  amjPacket p;
  Delays<float> m(NDELAYLINES);
  amjTime T;


  T.now();
  int packetType=CMD_MOVE;
  T.write(p.write(T.size()));
  p << m;
  s.send(p);
}

