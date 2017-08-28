#-------------------------------------------------
#
# Project created by QtCreator 2016-03-12T09:56:13
#
#-------------------------------------------------

QT       += core gui network

TARGET = Version1_0
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    posix_qextserialport.cpp \
    qextserialbase.cpp \
    qextserialport.cpp \
    serialclass.cpp \
    socketclass.cpp \
    moudle.cpp \
    ledandmotor.cpp \
    temperature.cpp \
    clock.cpp \
    replays.cpp \
    camerainit.cpp \
    airirda.cpp \
    smoke.cpp \
    moudleset.cpp \
    coorimpl.cpp \
    ultrasonicandpwm.cpp \
    login.cpp \
    WidgetKeyboard.cpp \
    curtain.cpp \
    detectusb.cpp \
    httpreply.cpp \
    download.cpp \
    security.cpp

HEADERS  += mainwindow.h \
    posix_qextserialport.h \
    qextserialbase.h \
    qextserialport.h \
    serialclass.h \
    socketclass.h \
    serialservice.h \
    abstracemoudle.h \
    moudle.h \
    ledandmotor.h \
    temperature.h \
    clock.h \
    replays.h \
    camerainit.h \
    airirda.h \
    smoke.h \
    moudleset.h \
    coorimpl.h \
    ultrasonicandpwm.h \
    login.h \
    WidgetKeyboard.hpp \
    curtain.h \
    detectusb.h \
    httpreply.h \
    download.h \
    security.h

FORMS    += mainwindow.ui \
    WidgetKeyboard.ui \
    login.ui

unix:DEFINES           += _TTY_POSIX_

OTHER_FILES +=
