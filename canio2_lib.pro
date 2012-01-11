TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = \
		plugins/plg_ixxat \
		plugins/plg_can_gate \
        tests/plugins_test \
        tests/test \
        py

QMAKE_CLEAN += Makefile
