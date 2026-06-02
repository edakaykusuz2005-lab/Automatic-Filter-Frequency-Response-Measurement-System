QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = STM32_BodeSweep_Qt5_11_Compatible
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialmanager.cpp \
    timercalculator.cpp \
    sweepcontroller.cpp \
    bodeplotwidget.cpp \
	protocol.cpp \
    dynamicbodeplotwidget.cpp

HEADERS += \
    mainwindow.h \
    serialmanager.h \
    timercalculator.h \
    sweepcontroller.h \
    sweeppoint.h \
    bodeplotwidget.h \
	protocol.h \
    dynamicbodeplotwidget.h

FORMS += \
    mainwindow.ui
