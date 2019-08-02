TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

unix: LIBS += -L$$OUT_PWD/../../src/ -lLockFreeQueue

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src

unix: PRE_TARGETDEPS += $$OUT_PWD/../../src/libLockFreeQueue.a

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += liburcu-cds liburcu
