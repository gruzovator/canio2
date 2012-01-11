# Common Config
CONFIG += dll
TEMPLATE = lib
# Target & dirs
TARGET = canio2
TARGET_EXT = .pyd
DESTDIR = ../bin_win
include(../canio2_lib.pri)
# Sources
INCLUDEPATH += . ../canio2 C:/Python27/include
SOURCES += \
    py.cpp \
    py_pmd.cpp \
    py_canopen.cpp
LIBS += -lC:/Python27/libs/python27
#
DEFINES += BOOST_PYTHON_STATIC_LIB

HEADERS += \
    ../canio2/triggers.hpp \
    ../canio2/pmd_node.hpp \
    ../canio2/node.hpp \
    ../canio2/io_service.hpp \
    ../canio2/canio2.hpp \
    ../canio2/canio_common.hpp \
    ../canio2/can_plugin.hpp \
    ../canio2/can_module_factories.hpp \
    ../canio2/can_module.hpp \
    ../canio2/detail/logger.hpp \
    ../canio2/detail/can_ext_module.hpp \
    ../canio2/co_node.hpp \
    py.hpp
