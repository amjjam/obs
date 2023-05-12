QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 debug

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.C \
    powerspectrumviewer.C \
    ../../shared/src/PowerSpectrum.C

HEADERS += \
    powerspectrumviewer.H

FORMS += \
    powerspectrumviewer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Tell the qcustomplot header that it will be used as library:
DEFINES += QCUSTOMPLOT_USE_LIBRARY

# Link with debug version of qcustomplot if compiling in debug mode, else with release library:
#CONFIG(debug, release|debug) {
#  win32:QCPLIB = qcustomplotd2
#  else: QCPLIB = qcustomplotd
#} else {
#  win32:QCPLIB = qcustomplot2
#  else: QCPLIB = qcustomplot
#}
QCPLIB=qcustomplot

LIBS += -L/opt/QCustomPlot/lib/ -l$$QCPLIB
INCLUDEPATH +=/opt/QCustomPlot/include
LD_LIBRARY_PATH +=/opt/QCustomPlot/lib

LIBS += -L/opt/amjCom/lib -lamjCom
INCLUDEPATH += /opt/amjCom/include
LD_LIBRARY_PATH +=/opt/amjCom/lib
