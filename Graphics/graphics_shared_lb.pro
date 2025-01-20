TEMPLATE = lib
TARGET = gp_gfx

QT += core gui widgets openglwidgets 

CONFIG += shared warn_off
CONFIG += debug_and_release
CONFIG += ordered

include(../../GridPro_Common/communication_lib.pri)
include(graphics_sources.pri)

win32 : {
    QMAKE_CXXFLAGS += /EHsc
    QMAKE_CXXFLAGS += /Zc:__cplusplus /std:c++17 /permissive-
}

viewer_variable += qt

DEFINES += BUILD_SHARED_LIB

# Destination
unix : !mac {
   DESTDIR = gp_gfx_shared_lib/lib/linux
}

mac : {
   DESTDIR = gp_gfx_shared_lib/lib/mac
}

win32 : {
   DESTDIR = gp_gfx_shared_lib/lib/windows
}


########################## End of Pro File ################################
# Build Directory segragation 
Release:OBJECTS_DIR = build/release/.obj
Release:MOC_DIR = build/release/.moc
Release:RCC_DIR = build/release/.rcc
Release:UI_DIR = build/release/.ui

Debug:OBJECTS_DIR = build/debug/.obj
Debug:MOC_DIR = build/debug/.moc
Debug:RCC_DIR = build/debug/.rcc
Debug:UI_DIR = build/debug/.ui

PRECOMPILED_DIR = build
QGLTF_DIR = build
TRACEGEN_DIR = build
QMLCACHE_DIR = build
LRELEASE_DIR = build
LEX_DIR = build
YACC_DIR = build
