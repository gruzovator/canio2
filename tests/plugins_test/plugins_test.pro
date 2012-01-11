# Common Config
CONFIG += console
TEMPLATE = app
# Target & dirs
TARGET = plugins_test
DESTDIR = ../../bin_win
include(../../canio2_lib.pri)
# Sources
INCLUDEPATH += . ../../canio2 ../
SOURCES += \
    plg_test.cpp
#
DEFINES += PROG_NAME=\\\"$$TARGET\\\" 

HEADERS += \
    ../tests_common.hpp
