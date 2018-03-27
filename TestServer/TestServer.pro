include (../Open62541.pri)
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -L $$INSTALLBASE/lib -lOpen62541Cpp
INCLUDEPATH += $$INSTALLBASE/include/Open62541Cpp
SOURCES += main.cpp \
    testcontext.cpp \
    testmethod.cpp \
    testobject.cpp

HEADERS += \
    testcontext.h \
    testmethod.h \
    testobject.h

