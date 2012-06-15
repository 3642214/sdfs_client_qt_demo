#-------------------------------------------------
#
# Project created by QtCreator 2012-06-09T14:48:20
#
#-------------------------------------------------

QT       += core gui

TARGET = skySDFS
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    workthread.cpp

HEADERS  += mainwindow.h \
    SkySDFSSDK.h \
    workthread.h

FORMS    += mainwindow.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../release/ -lclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../debug/ -lclient
else:unix: LIBS += ./libclient.so

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../

OTHER_FILES += \
    testReadFile
