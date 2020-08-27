INCLUDEPATH += ./../../../include
INCLUDEPATH += ./../../../Lib
LIBS += -L"./../../../Lib" -lIBScanUltimate

TEMPLATE = app
TARGET = IBScanUltimate_SampleForQT
DESTDIR = ../../../Build
QT += core gui

HEADERS += ./IBSU_SampleForQT.h ./TypeReDef.h
SOURCES += ./IBSU_SampleForQT.cpp ./main.cpp
FORMS += ./IBSU_SampleForQT.ui
RESOURCES += ./Images.qrc

CONFIG += static
CONFIG += release
CONFIG += warn_off
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
QMAKE_LFLAGS += -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive

static {
CONFIG += static
DEFINES += STATIC
message("Static build.")
}