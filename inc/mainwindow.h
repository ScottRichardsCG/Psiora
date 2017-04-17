#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void* getDrawingArea();

private slots:
	void closeEvent(QCloseEvent *event);
	void on_actionHardReset_triggered();
    void on_actionLoad_ROM_triggered();
	void on_actionSwitch_On_triggered();
	void do_everySecond();
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

	void on_actionInsertPakB_triggered();
	void on_actionInsertPakC_triggered();
	void on_actionEjectPakB_triggered();
	void on_actionEjectPakC_triggered();

	void insertPak(int id);
	void ejectPak(int id);

private:
	void popupDialog(QString mainText, QString infoText, QMessageBox::Icon icon, QString butText);
	bool popupDialogCustom(QString mainText, QString infoText, QMessageBox::Icon icon, QString butText);

    Ui::MainWindow *ui;
	QTimer *everySecondTimer;
};

#endif // MAINWINDOW_H
