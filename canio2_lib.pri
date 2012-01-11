CONFIG -= qt debug_and_release debug_and_release_target
BOOST = C:/boost_1_46_1
TMP = tmp
CONFIG(debug, debug|release){
    #TARGET = $$join(TARGET,,,_d)
	OBJECTS_DIR = $$TMP/debug
    DEFINES += DEBUG
} else {
	OBJECTS_DIR = $$TMP/release
    DEFINES -= DEBUG
}
win32:QMAKE_CLEAN += Makefile & rd /s /q $$TMP
INCLUDEPATH += . $$BOOST
LIBS += -L$$BOOST/lib

QMAKE_CXXFLAGS -= -Zc:wchar_t-

HEADERS += \
    ../../canio2/co_node.hpp
