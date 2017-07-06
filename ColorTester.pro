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

INCLUDEPATH += src

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/viewwidget.cpp \
    src/matrixeditor.cpp \
    src/rotationeditor.cpp \
    src/quaternion.cpp \
    src/vector3.cpp \
    src/pigmenteditor.cpp

HEADERS  += src/mainwindow.h \
    src/viewwidget.h \
    src/matrixeditor.h \
    src/rotationeditor.h \
    src/quaternion.h \
    src/vector3.h \
    src/pigmenteditor.h

FORMS    += src/mainwindow.ui \
    src/matrixeditor.ui \
    src/rotationeditor.ui \
    src/pigmenteditor.ui
