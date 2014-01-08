TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    XCOMContainer.cpp

HEADERS += \
    upp11.h \
    XCOMContainer.h

QMAKE_CXXFLAGS += -std=c++11

