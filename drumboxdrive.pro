TEMPLATE    = app

SOURCES     = jack_engine.cpp \
drumboxdrive.cpp \
PortLatency.cpp \
Bridge.cpp \
main.cpp

HEADERS     = jack_engine.h \
drumboxdrive.h \
Bridge.h \
PortLatency.h

INCLUDEPATH += qextserialport/src/
INCLUDEPATH += /usr/include/
INCLUDEPATH += /usr/local/include/

LIBS += -ljack -lqjack -ludev -Lqextserialport/ -lqextserialport

QT += xml

target.path += /usr/bin/
INSTALLS += target

CONFIG += debug

DESTDIR=bin
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build

RESOURCES= application.qrc
