# Common Config
CONFIG += dll
TEMPLATE = lib
# Target & dirs
TARGET = plg_ixxat
DESTDIR = ../../bin_win
include(../../canio2_lib.pri)
# Sources
INCLUDEPATH += . $$BOOST vendor ../../canio2
SOURCES += ixxat.cpp
LIBS += -Lvendor -lvcisdk
DEFINES += CAN_PLUGIN
