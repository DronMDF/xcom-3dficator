TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    XCOMContainer.cpp \
    XCOMContainerPCK.cpp

HEADERS += \
    upp11.h \
    XCOMContainer.h \
    XCOMContainerPCK.h

QMAKE_CXXFLAGS += -std=c++11
LIBS += -lboost_system -lboost_filesystem
