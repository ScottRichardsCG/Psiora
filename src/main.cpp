#include "mainwindow.h"
#include <QApplication>
#include "emucore.h"
#include "global.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();

    emucore = new EmuCore(mainWin.getDrawingArea());
    emucore->startCore();

    app.exec();

    emucore->stopCore();
    delete emucore;

    return 0;
}
