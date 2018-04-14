#include "keycustomize.h"
#include "ui_keycustomize.h"

KeyCustomize::KeyCustomize(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyCustomize)
{
    ui->setupUi(this);
}

KeyCustomize::~KeyCustomize()
{
    delete ui;
}

void KeyCustomize::accept()
{
    QDialog::accept();

    emit acceptedKey(ui->key->text());
}
