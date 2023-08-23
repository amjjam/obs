QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.C \
    FrameViewer.C

HEADERS += \
    FrameViewer.H

FORMS += \
    FrameViewer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QCPLIB=qcustomplot

LIBS += -L/opt/QCustomPlot/lib/ -l$$QCPLIB
INCLUDEPATH +=/opt/QCustomPlot/include
LD_LIBRARY_PATH +=/opt/QCustomPlot/lib

LIBS += -L/opt/amjCom/lib -lamjCom
INCLUDEPATH += /opt/amjCom/include
LD_LIBRARY_PATH +=/opt/amjCom/lib

LIBS += -L/opt/amj/lib -lamjFourier
INCLUDEPATH += /opt/amj/include
LD_LIBRARY_PATH += /opt/amj/lib
