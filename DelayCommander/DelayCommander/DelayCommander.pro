QT       += core gui

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

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.C \
    DelayCommander.C

HEADERS += \
    DelayCommander.H

FORMS += \
    DelayCommander.ui

LIBS += $${LIB} -lamjCom

INCLUDEPATH += $${INC}

# Default rules for deployment.
target.path = $${prefix}/bin
INSTALLS += target
