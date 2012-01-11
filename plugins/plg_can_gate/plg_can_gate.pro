# Common Config
CONFIG += dll
TEMPLATE = lib
# Target & dirs
TARGET = plg_can_gate
DESTDIR = ../../bin_win
include(../../canio2_lib.pri)
# Sources
INCLUDEPATH += . $$BOOST vendor ../../canio2
SOURCES += plg_can_gate.cpp
LIBS += -lws2_32
DEFINES += CAN_PLUGIN
