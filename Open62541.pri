# Raspberry Pi cross compile
unix {
    options = $$find(QMAKESPEC, "rpi")
    count(options,1) {
        INSTALLBASE = $$[QT_SYSROOT]/usr/local
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
    INSTALLBASE = C:/usr/local
    LIBS += -LC:/local/boost_1_66_0/lib32-msvc-12.0 -lboost_system-vc120-mt-x32-1_66 -lWs2_32
    INCLUDEPATH += C:/local/boost_1_66_0

}



