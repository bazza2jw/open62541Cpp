#-------------------------------------------------
#
# Project created by QtCreator 2017-10-12T10:37:20
#
#-------------------------------------------------
include (Open62541.pri)

QT       -= core gui
unix {
QMAKE_CFLAGS += -std=c99
}
CONFIG += c++11 thread

TARGET = Open62541Cpp
TEMPLATE = lib

# for Win32
DEFINES += UA_DYNAMIC_LINKING_EXPORT

INCLUDEPATH += $$PWD/include

SOURCES += src/open62541client.cpp \
    src/open62541.c \
    src/open62541server.cpp \
    src/open62541objects.cpp \
    src/clientcache.cpp \
    src/clientcachethread.cpp \
    src/nodecontext.cpp \
    src/serverrepeatedcallback.cpp \
    src/servermethod.cpp \
    src/serverobjecttype.cpp \
    src/clientbrowser.cpp \
    src/monitoreditem.cpp \
    src/clientsubscription.cpp
    #src/monitoreditem.cpp \
    #src/clientsubscription.cpp

HEADERS +=\
    include/open62541client.h \
    include/open62541objects.h \
    include/open62541.h \
    include/open62541server.h \
    include/propertytree.h \
    include/clientcache.h \
    include/clientcachethread.h \
    include/nodecontext.h \
    include/servermethod.h \
    include/serverrepeatedcallback.h \
    include/serverobjecttype.h \
    include/serverbrowser.h \
    include/servernodetree.h \
    include/clientnodetree.h \
    include/clientbrowser.h \
    include/monitoreditem.h \
    include/clientsubscription.h
#    include/monitoreditem.h \
#    include/clientsubscription.h \


headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/Open62541Cpp
target.path = $$INSTALLBASE/lib

INSTALLS += headers
INSTALLS += target


