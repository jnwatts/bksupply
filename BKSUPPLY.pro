#-------------------------------------------------
#
# Project created by QtCreator 2017-08-04T10:28:04
#
#-------------------------------------------------

QT       += core gui serialport
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bksupply
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    bkserial.cpp \
    bk1696.cpp

HEADERS  += mainwindow.h \
    bkserial.h \
    bk1696.h

FORMS    += mainwindow.ui

isEmpty(PREFIX) {
	PREFIX = /usr/local
}

target.path = $$PREFIX/bin
INSTALLS += target
