#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
	void on_actionHardReset_triggered();
    void on_actionLoad_ROM_triggered();
	void on_actionSwitch_On_triggered();
	void do_everySecond();
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
	QTimer *everySecondTimer;
};

#endif // MAINWINDOW_H
