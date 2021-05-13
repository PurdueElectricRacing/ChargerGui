QT += core gui widgets

TARGET = ChargerGui

SOURCES += main.cpp \
           chargergui.cpp 

HEADERS += chargergui.h \
    timer.h

FORMS += chargergui.ui

DEFINES += STUB_CAN # comment this line when building for reals

include(./DesktopCAN_API/canapi.pri)