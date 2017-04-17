#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>

namespace Ui {
class CreatePakDialog;
}

class CreatePakDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatePakDialog(QWidget *parent = 0);
    ~CreatePakDialog();

private slots:

private:
    Ui::CreatePakDialog *ui;
};

#endif // MAINWINDOW_H
