#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include <QPushButton>

#include "emucore.h"
#include "keypad.h"
#include "datapak.h"

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

void MainWindow::closeEvent(QCloseEvent *event) {
	emucore->pause(true);
	int suc = emucore->save();
	if (suc == FILEIO_SUCCESS) {
		return;
	}

	QString message = "An error occured while trying to save RAM. Changes to RAM cannot be saved at this time.";
	if (suc == FILEIO_SaveFail_PowerOn) {
		message = "The emulator is not switched off. Changes to RAM cannot be saved at this time.";
	}
	bool ret = popupDialogCustom(message, "Are you sure you want to quit anyway?", QMessageBox::Warning, "Quit Anyway");
	if (!ret) {
		emucore->pause(false);
		event->ignore();
		return;
	}
}

bool MainWindow::popupDialogCustom(QString mainText, QString infoText, QMessageBox::Icon icon, QString butText) {
	QMessageBox msgBox;
	msgBox.setText(mainText);
	msgBox.setInformativeText(infoText);
	msgBox.setIcon(icon);
	msgBox.setStandardButtons(QMessageBox::Cancel);
	QPushButton *button = msgBox.addButton(butText, QMessageBox::ActionRole);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	msgBox.exec();
	if (msgBox.clickedButton() == button) {
		return true;
	}
	return false;
}

void MainWindow::popupDialog(QString mainText, QString infoText, QMessageBox::Icon icon, QString butText) {
	QMessageBox msgBox;
	msgBox.setText(mainText);
	msgBox.setInformativeText(infoText);
	msgBox.setIcon(icon);
	msgBox.addButton(butText, QMessageBox::ActionRole);
	msgBox.exec();
}

void* MainWindow::getDrawingArea()
{
    return (void*) ui->widget->winId();
}

void MainWindow::on_actionLoad_ROM_triggered()
{
	emucore->pause(true);

	if (emucore->isPowered()) {
		bool suc = popupDialogCustom("The emulator is not switched off. Changes to RAM cannot be saved as this time.",
			"Are you sure you want to load a new ROM anyway?",
			QMessageBox::Warning,
			"Load Anyway");


		QMessageBox msgBox;
		msgBox.setText("The emulator is not switched off. Changes to RAM cannot be saved as this time.");
		msgBox.setInformativeText("Are you sure you want to load a new ROM anyway?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		msgBox.setIcon(QMessageBox::Warning);
		int ret = msgBox.exec();
		if (ret == QMessageBox::Cancel) {
			emucore->pause(false);
			return;
		}
	}
	
	int suc = emucore->save();
	if (suc != FILEIO_SUCCESS) {
		QMessageBox msgBox;
		msgBox.setText("Problem saving the current RAM. Loading a new ROM with result in loss of data.");
		msgBox.setInformativeText("Are you sure you want to load a new ROM anyway?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		msgBox.setIcon(QMessageBox::Warning);
		int ret = msgBox.exec();
		if (ret == QMessageBox::Cancel) {
			emucore->pause(false);
			return;
		}
	}

    QString fileName = QFileDialog::getOpenFileName(this, "Select ROM file...", "", "Psion ROMs (*.bin *.rom *.dat);;All Files (*.*)");
    if (fileName == "") {
		emucore->pause(false);
        return;
    }
    
	suc = emucore->load(fileName.toStdString());
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

void MainWindow::on_actionHardReset_triggered() {
	emucore->pause(true);
	QMessageBox msgBox;
	msgBox.setText("This will completely erase all data in this ROM. This is unrecoverable!!");
	msgBox.setInformativeText("Are you sure you want to hard reset the emulator?");
	msgBox.setStandardButtons(QMessageBox::Cancel);
	QPushButton *hardResetButton = msgBox.addButton("Hard Reset!", QMessageBox::ActionRole);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	msgBox.exec();
	if (msgBox.clickedButton() == hardResetButton) {
		emucore->hardReset();
	}
	emucore->pause(false);
}

void MainWindow::on_actionSwitch_On_triggered() {
	emucore->setPower(true);
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

void MainWindow::insertPak(int id) {
	emucore->pause(true);

	QString fileName = QFileDialog::getOpenFileName(this, "Select OPK file to insert...", "", "OPK PAK Images (*.opk);All Files (*.*)");
	if (fileName == "") {
		emucore->pause(false);
		return;
	}

	int suc = emucore->insertPak(id, fileName.toStdString());
	if (suc != 0) {
		QMessageBox msgBox;
		switch (suc) {
		case PAK_ERR_CANT_OPEN:
			msgBox.setText("PAK image could not be loaded");
			break;
		case PAK_ERR_NOT_OPK:
			msgBox.setText("Not an OPK type PAK image");
			break;
		case PAK_ERR_TOO_LARGE:
			msgBox.setText("PAK too large. PAKs larger than 64KB are not currently supported.");
			break;
		default:
			msgBox.setText("Unknown error occured: " + QString::number(suc));
			break;
		}
		msgBox.exec();
	}
	emucore->pause(false);
}

void MainWindow::ejectPak(int id) {
	emucore->pause(true);
	if (!emucore->ejectPak(id, false)) {
		QMessageBox msgBox;
		msgBox.setText("Pak data could not be saved");
		msgBox.setInformativeText("Do you want to eject pak anyway?");
		msgBox.setStandardButtons(QMessageBox::Cancel);
		QPushButton *eject = msgBox.addButton("Eject Anyway", QMessageBox::ActionRole);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		msgBox.exec();
		if (msgBox.clickedButton() == eject) {
			emucore->ejectPak(id, true);
		}
	}
	emucore->pause(false);
}

void MainWindow::on_actionInsertPakB_triggered() {
	insertPak(1);
}

void MainWindow::on_actionInsertPakC_triggered() {
	insertPak(0);
}

void MainWindow::on_actionEjectPakB_triggered() {
	ejectPak(1);
}

void MainWindow::on_actionEjectPakC_triggered() {
	ejectPak(0);
}