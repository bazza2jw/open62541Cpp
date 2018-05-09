# Raspberry Pi cross compile
unix {
    options = $$find(QMAKESPEC, "rpi")
    count(options,1) {
        INSTALLBASE = /opt/RPiSysroot2/usr/local
        message("Raspberry Pi Build")
        LIBS += -L$$[QT_SYSROOT]/usr/local/lib -L$$[QT_SYSROOT]/lib/arm-linux-gnueabihf -L$$[QT_SYSROOT]/usr/lib/arm-linux-gnueabihf
        LIBS += -lboost_system -lboost_thread
        DEFINES += RASPBERRY_PI_BUILD
        CONFIG += raspi
        INCLUDEPATH += /opt/RPiSysroot2/usr/include/arm-linux-gnueabihf
        INCLUDEPATH += /opt/RPiSysroot2/usr/include


    } else {
        message("Linux PC Build")
# where to install library and includes - change to suit
        INSTALLBASE  = /usr/local/AMPI
        LIBS += -lboost_system -lboost_thread
    }

}

win32 {
# Set location of Boost Libraries
    message("MS Windows PC Build")
    INSTALLBASE = D:/usr/local
    LIBS += -LD:/usr/local/boost_1_67_0/lib64-msvc-14.1 -lboost_system-vc141-mt-x64-1_67 -lWs2_32
    INCLUDEPATH += D:/usr/local/boost_1_67_0
    DEFINES += UA_DYNAMIC_LINKING
}



