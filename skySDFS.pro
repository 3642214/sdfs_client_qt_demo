#-------------------------------------------------
#
# Project created by QtCreator 2012-06-09T14:48:20
#
#-------------------------------------------------

QT       += core gui

TARGET = skySDFS
TEMPLATE = app

win32:LIBS += -L$$PWD/ -lskyFS-client-cD
else:unix:LIBS += -L$$PWD/ -lclient

SOURCES += main.cpp\
        mainwindow.cpp \
    workthread.cpp

HEADERS  += mainwindow.h \
    SkySDFSSDK.h \
    workthread.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    testReadFile \
    config.ini
