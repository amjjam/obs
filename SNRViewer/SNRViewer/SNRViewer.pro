QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

isEmpty(prefix){
    prefix=/opt/obs
}
isEmpty(INC){
    INC=/opt/amj/include /opt/QCustomPlot/include
}
isEmpty(LIB){
    LIB=-L/opt/amj/lib -L/opt/QCustomPlot/lib
}

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.C SNRViewer.C ../../shared/src/Help.C

HEADERS += SNRViewer.H

FORMS += SNRViewer.ui

LIBS+= $${LIB} -lqcustomplot -lamjCom -lamjChart -lamjTime

INCLUDEPATH += $${INC}


# Default rules for deployment.
target.path=$${prefix}/bin
!isEmpty(target.path): INSTALLS += target
