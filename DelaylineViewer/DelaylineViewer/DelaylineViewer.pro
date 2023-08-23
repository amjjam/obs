QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.C \
    DelaylineViewer.C \
    ../../shared/src/Help.C

HEADERS += \
    DelaylineViewer.H \
    ../../shared/include/Delays.H \
    ../../shared/include/Help.H

FORMS += \
    DelaylineViewer.ui

LIBS += -L/opt/QCustomPlot/lib -lqcustomplot -L/opt/amjCom/lib -lamjCom -L/opt/amjChart/lib -lamjChart

INCLUDEPATH += /opt/amjChart/include /opt/amjCom/include /opt/QCustomPlot/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

