include (../../Common.pri)
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
DESTDIR=$$APPDIR
LIBS += -lDWTRuntime -lDWTData -lOpcServiceCommon
SOURCES += main.cpp \
    simulatorapp.cpp \
    simulatoropc.cpp \
    simulatornodecontext.cpp \
    simulatorstartmethod.cpp \
    simulatorstopmethod.cpp \
    simulateprocess.cpp

HEADERS += \
    simulatorapp.h \
    Simulator.h \
    simulatoropc.h \
    simulatordefs.h \
    simulatornodecontext.h \
    simulatorstartmethod.h \
    simulatorstopmethod.h \
    simulateprocess.h
