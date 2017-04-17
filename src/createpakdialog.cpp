#include "createpakdialog.h"
#include "ui_createpakdialog.h"

CreatePakDialog::CreatePakDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatePakDialog)
{
    ui->setupUi(this);
}

CreatePakDialog::~CreatePakDialog()
{
    delete ui;
}
