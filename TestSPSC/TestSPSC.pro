TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        container.c \
        spsc_test_main.c

HEADERS += \
        container.h

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += liburcu-cds liburcu

unix: QMAKE_CFLAGS += -pthread
unix: QMAKE_CXXFLAGS += -pthread
unix: QMAKE_LFLAGS += -pthread
