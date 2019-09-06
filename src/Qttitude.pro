#-------------------------------------------------
#
# Project created by QtCreator 2017-09-30T00:33:01
#
#-------------------------------------------------

QT       += core gui uitools
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Qttitude
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    coloreditorwidget.cpp \
    highlighter.cpp \
    stylesheeteditoritemdelegate.cpp \
    stylesheeteditorwidget.cpp \
    texteditor.cpp \
    colorschemegenerator.cpp \
    workspace.cpp \
    dialogcolorspec.cpp \
    globals.cpp \
    dragitemmodel.cpp \
    extendedlineedit.cpp \
    dialogtexteditor.cpp \
    project.cpp \
    codegen.cpp \
    dialogpagecreator.cpp

HEADERS  += mainwindow.h \
    coloreditorwidget.h \
    highlighter.h \
    stylesheeteditoritemdelegate.h \
    stylesheeteditorwidget.h \
    texteditor.h \
    colorschemegenerator.h \
    workspace.h \
    dialogcolorspec.h \
    globals.h \
    third_party/dkm/include/dkm.hpp \
    dragitemmodel.h \
    extendedlineedit.h \
    dialogtexteditor.h \
    project.h \
    codegen.h \
    dialogpagecreator.h

FORMS    += mainwindow.ui \
    stylesheeteditorwidget.ui \
    dialogcolorspec.ui \
    form.ui \
    dialogtexteditor.ui \
    dialogpagecreator.ui

RESOURCES += \
    resources.qrc
