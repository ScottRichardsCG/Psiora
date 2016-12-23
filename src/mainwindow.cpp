#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QMessageBox>
#include "emucore.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->widget->setUpdatesEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void* MainWindow::getDrawingArea()
{
    return (void*) ui->widget->winId();
}

void MainWindow::on_actionLoad_ROM_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select ROM file...", "", "Psion ROMs (*.bin *.rom *.dat);;All Files (*.*)");
    if (fileName == "") {
        return;
    }
    std::cerr << fileName.toStdString();
    int suc = emucore->load(fileName);
    if (suc == FILEIO_ROM_LoadFail) {
        QMessageBox msgBox;
        msgBox.setText("ROM could not be loaded");
        msgBox.exec();
        return;
    } else if (suc == FILEIO_ROM_Invalid) {
        QMessageBox msgBox;
        msgBox.setText("Not a valid ROM file");
        msgBox.exec();
        return;
    }
    emucore->setPower(true);
}
