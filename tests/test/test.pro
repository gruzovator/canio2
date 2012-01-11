# Common Config
CONFIG += console
TEMPLATE = app
# Target & dirs
TARGET = test
DESTDIR = ../../bin_win
include(../../canio2_lib.pri)
# Sources
INCLUDEPATH += . ../../canio2 ../
SOURCES += \
    test.cpp
#
DEFINES += PROG_NAME=\\\"$$TARGET\\\" 

HEADERS += \
    ../tests_common.hpp
