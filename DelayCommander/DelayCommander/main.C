#include "DelayCommander.H"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  DelayCommander w;
  w.show();
  return a.exec();
}
