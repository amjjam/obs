QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

isEmpty(prefix){
  prefix=/opt/obs
}
isEmpty(INC){
  INC=/opt/amj/include /opt/amj/include/amjCom/ /opt/QCustomPlot/include /opt/asio/include
}
isEmpty(LIB){
  LIB=-L/opt/amj/lib -L/opt/QCustomPlot/lib
}


CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.C \
    DataProcessorGUI.C \
    ../../DataProcessor/src/DataProcessorStatus.C

HEADERS += DataProcessorGUI.H

FORMS += DataProcessorGUI.ui

LIBS += $${LIB} -lamjCom -lamjTime

INCLUDEPATH += $${INC}

# Default rules for deployment.
target.path=$${prefix}/bin
!isEmpty(target.path): INSTALLS += target
