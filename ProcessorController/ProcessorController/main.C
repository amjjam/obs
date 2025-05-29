#include "ProcessorController.H"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  ProcessorController w;
  w.show();
  return a.exec();
}
