#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::connectSlots(MainWorker *worker){
    connect(ui->rescanButton, SIGNAL(clicked()), worker, SLOT(doRescan()));
}

void MainWindow::onUpdateList(QList<QString> list){

}
