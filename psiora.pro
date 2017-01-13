#-------------------------------------------------
#
# Project created by QtCreator 2016-04-04T23:06:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = psiora
TEMPLATE = app


#Inclue SDL1.2 - Win32
win32:DEFINES += "_GNU_SOURCE=1"
#win32:DEFINES += "main=SDL_main"

#Include SDL1.2 - Win32 Mingw
win32-g++*:INCLUDEPATH += C:/Devel/SDL-devel-1.2.15-mingw32/include/SDL
#win32-g++*:LIBS += C:/Devel/SDL-devel-1.2.15-mingw32/lib/libSDLmain.a
win32-g++*:LIBS += C:/Devel/SDL-devel-1.2.15-mingw32/lib/libSDL.dll.a

#Include SDL1.2 - Win32 VC
win32-msvc*:INCLUDEPATH += C:/Devel/SDL-devel-1.2.15-VC/include
win32-msvc*:LIBS += winmm.lib
#win32-msvc*:LIBS += C:/Devel/SDL-devel-1.2.15-VC/lib/x86/SDLmain.lib
win32-msvc*:LIBS += C:/Devel/SDL-devel-1.2.15-VC/lib/x86/SDL.lib

#Include SDL1.2 - Unix
unix:INCLUDEPATH += /usr/include/SDL2
unix:LIBS += -L/usr/lib/x86_64-linux-gnu -lSDL2
unix:QMAKE_CXXFLAGS += -I/usr/include/SDL2 -D_REENTRANT -std=c++11
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
    src/debug.cpp

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
    inc/debug.h

FORMS    += forms/mainwindow.ui

DISTFILES +=

RESOURCES += \
    resources.qrc
		
