#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T09:10:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ColorTester
TEMPLATE = app

LIBS += -ldrm

SOURCES += main.cpp\
        mainwindow.cpp \
    viewwidget.cpp \
    matrixeditor.cpp \
    rotationeditor.cpp

HEADERS  += mainwindow.h \
    viewwidget.h \
    matrixeditor.h \
    rotationeditor.h

FORMS    += mainwindow.ui \
    matrixeditor.ui \
    rotationeditor.ui
