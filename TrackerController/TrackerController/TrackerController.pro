
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

isEmpty(prefix){
  prefix=/opt/obs
}
isEmpty(INC){
  INC=/opt/amj/include
}
isEmpty(LIB){
  LIB=-L/opt/amj/lib
}

CONFIG += c++11 debug

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += ../../FringeTracker/src/FringeTrackerStateMachine.C \
    main.C TrackerController.C

HEADERS += TrackerController.H

FORMS += TrackerController.ui

LIBS += $${LIB} -lamjCom -lamjWidgets

INCLUDEPATH += $${INC}
        
# Default rules for deployment.
target.path = $${prefix}/bin
INSTALLS += target
