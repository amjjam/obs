#include "TrackerController.H"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  TrackerController w;
  w.show();
  return a.exec();
}
