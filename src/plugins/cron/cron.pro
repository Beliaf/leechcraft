######################################################################
# Automatically generated by qmake (2.01a) Thu Dec 20 20:59:12 2007
######################################################################

TEMPLATE = lib
CONFIG += plugin release
TARGET = leechcraft_cron
DESTDIR = ../bin
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../../
QT -= gui

# Input
HEADERS += cron.h core.h globals.h
SOURCES += cron.cpp core.cpp
RESOURCES += resources.qrc
win32:LIBS += -L../.. -lplugininterface