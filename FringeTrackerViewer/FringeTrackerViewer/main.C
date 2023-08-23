#include "FringeTrackerViewer.H"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  FringeTrackerViewer w;
  w.show();
  return a.exec();
}
