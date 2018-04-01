#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "zlog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scanning(false)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::connectWorker(MainWorker *worker){
    connect(this, SIGNAL(doRescan()), worker, SLOT(onDoRescan()));
    connect(worker, SIGNAL(rescanDone(QStringList)), this, SLOT(onRescanDone(QStringList)));
}

void MainWindow::on_rescanButton_clicked(){
    if(!scanning){
        scanning = true;
        emit doRescan();
        ui->rescanButton->setEnabled(false);
        ui->keyboardSelect->clear();
        ui->keyboardSelect->setEnabled(false);
        ui->progressBar->setMaximum(0);
        ui->statusBar->showMessage("Scanning...");
    }
}

void MainWindow::onRescanDone(QStringList list){
    scanning = false;
    ui->rescanButton->setEnabled(true);
    ui->keyboardSelect->addItems(list);
    ui->keyboardSelect->setEnabled(true);
    ui->progressBar->setValue(100);
    ui->progressBar->setMaximum(100);
    ui->statusBar->showMessage("Scan Done");
}
