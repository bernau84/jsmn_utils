#-------------------------------------------------
#
# Project created by QtCreator 2015-04-01T13:00:27
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = json_io_tests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    json_utils_test.cpp \
    json_utils.cpp \
    jsmn.c \
    main_qt.cpp

HEADERS += \
    json_utils.h \
    jsmn.h
