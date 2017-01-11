#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include "emucore.h"
#include "keypad.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->widget->setUpdatesEnabled(false);

	everySecondTimer = new QTimer(this);
	connect(everySecondTimer, SIGNAL(timeout()), this, SLOT(do_everySecond()));
	everySecondTimer->start(1000);
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
	emucore->pause(true);
    QString fileName = QFileDialog::getOpenFileName(this, "Select ROM file...", "", "Psion ROMs (*.bin *.rom *.dat);;All Files (*.*)");
    if (fileName == "") {
		emucore->pause(false);
        return;
    }
    int suc = emucore->load(fileName.toStdString());
    if (suc == FILEIO_ROM_LoadFail) {
        QMessageBox msgBox;
        msgBox.setText("ROM could not be loaded");
        msgBox.exec();
		emucore->pause(false);
        return;
    } else if (suc == FILEIO_ROM_Invalid) {
        QMessageBox msgBox;
        msgBox.setText("Not a valid ROM file");
        msgBox.exec();
		emucore->pause(false);
        return;
    }
    emucore->setPower(true);
	emucore->pause(false);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
	if (event->isAutoRepeat() == false) {
		switch (event->key()) {
		case Qt::Key_F1:
			keypad->keyDown(KEYSTATE_F1); break;
		case Qt::Key_F2:
			keypad->keyDown(KEYSTATE_F2); break;
		case Qt::Key_Shift:
			keypad->keyDown(KEYSTATE_SHIFT); break;
		case Qt::Key_Up:
			keypad->keyDown(KEYSTATE_UP); break;
		case Qt::Key_Down:
			keypad->keyDown(KEYSTATE_DOWN); break;
		case Qt::Key_Left:
			keypad->keyDown(KEYSTATE_LEFT); break;
		case Qt::Key_Right:
			keypad->keyDown(KEYSTATE_RIGHT); break;
		case Qt::Key_Space:
			keypad->keyDown(' '); break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			keypad->keyDown(KEYSTATE_RETURN); break;
		case Qt::Key_Backspace:
			keypad->keyDown(KEYSTATE_BACKSPACE); break;
		case Qt::Key_Delete:
			keypad->keyDown(KEYSTATE_DELETE); break;
		default:
			keypad->keyDown(event->text().toStdString().c_str()[0]);
		}
	}
	event->ignore();    /// Leave emulator in control of all keyboard events
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
	if (event->isAutoRepeat() == false)
		keypad->keyUp(event->key());
	event->ignore();    /// Leave emulator in control of all keyboard events
}

void MainWindow::do_everySecond() {
	ui->speed->setValue(emucore->getEmuSpeed());
	ui->speed->setFormat("%p%");
}