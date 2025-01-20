TEMPLATE = app
TARGET = ws_imgui
QT -= core gui widgets

CONFIG += desktop
CONFIG += console
CONFIG += debug_and_release

viewer_variable += imgui

include(../graphics_sources.pri)

SOURCES += imgui_app.cpp

DESTDIR = $$PWD

# Library settings per platform
unix {
    # Linux
    LIBS += -lGL -lGLEW -lglfw
}

########################## End of Pro File ################################
# Build Directory segragation 
Release:OBJECTS_DIR = $$PWD/build/release/.obj
Release:MOC_DIR = $$PWD/build/release/.moc
Release:RCC_DIR = $$PWD/build/release/.rcc
Release:UI_DIR = $$PWD/build/release/.ui

Debug:OBJECTS_DIR = $$PWD/build/debug/.obj
Debug:MOC_DIR = $$PWD/build/debug/.moc
Debug:RCC_DIR = $$PWD/build/debug/.rcc
Debug:UI_DIR = $$PWD/build/debug/.ui

PRECOMPILED_DIR = $$PWD/build
QGLTF_DIR = $$PWD/build
TRACEGEN_DIR = $$PWD/build
QMLCACHE_DIR = $$PWD/build
LRELEASE_DIR = $$PWD/build
LEX_DIR = $$PWD/build
YACC_DIR = $$PWD/build