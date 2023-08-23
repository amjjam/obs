#include "FrameViewer.H"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  FrameViewer w;
  w.show();
  return a.exec();
}
