#include "DataProcessorGUI.H"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  DataProcessorGUI w;
  w.show();
  return a.exec();
}
