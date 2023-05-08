#include "powerspectrumviewer.H"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  PowerSpectrumViewer w;
  w.show();
  return a.exec();
}

