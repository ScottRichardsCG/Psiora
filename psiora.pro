#-------------------------------------------------
#
# Project created by QtCreator 2016-04-04T23:06:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = psiora
TEMPLATE = app


#Inclue SDL2 - Win32
win32:DEFINES += "_GNU_SOURCE=1"

#Include SDL2 - Win32 Mingw
win32-g++*:INCLUDEPATH += %SDL2DIR%/i686-w64-mingw32/include/SDL2
win32-g++*:LIBS += %SDL2DIR%/i686-w64-mingw32/lib/libSDL2.dll.a

#Include SDL2 - Win32 VC
win32-msvc*:INCLUDEPATH += %SDL2DIR%/include
win32-msvc*:LIBS += winmm.lib
win32-msvc*:LIBS += %SDL2DIR%/lib/x86/SDL2.lib

#Include SDL2 - Unix
unix:INCLUDEPATH += /usr/include/SDL2
unix:LIBS += `sdl2-config --libs`
unix:QMAKE_CXXFLAGS += `sdl2-config --cflags` -std=c++11
unix:QMAKE_CXXFLAGS -= "-Dmain=SDL_main"

INCLUDEPATH += inc/ src/

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/renderer.cpp \
    src/keypad.cpp \
    src/emucore.cpp \
    src/memory.cpp \
    src/scic.cpp \
    src/cpu.cpp \
    src/datapak.cpp \
    src/cpu_opcodes.cpp \
    src/lcd.cpp \
    src/debug.cpp \
		src/createpakdialog.cpp

HEADERS  += inc/mainwindow.h \
    inc/renderer.h \
    inc/keypad.h \
    inc/emucore.h \
    inc/global.h \
    inc/memory.h \
    inc/scic.h \
    inc/cpu.h \
    inc/datapak.h \
    inc/lcd.h \
    inc/debug.h \
		inc/createpakdialog.h

FORMS    += forms/mainwindow.ui \
		forms/createpakdialog.ui

DISTFILES +=

RESOURCES += \
    resources.qrc
		
