#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "zlog.h"

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

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
    connect(worker, SIGNAL(rescanDone(ZList<KeyboardDevice>)), this, SLOT(onRescanDone(ZList<KeyboardDevice>)));
}

void MainWindow::on_rescanButton_clicked(){
    if(!scanning){
        scanning = true;
        emit doRescan();
        ui->rescanButton->setEnabled(false);
        ui->progressBar->setMaximum(0);
        ui->statusBar->showMessage("Scanning...");
        ui->keyboardSelect->clear();
        ui->keyboardSelect->setEnabled(false);
        ui->flash->setEnabled(false);
    }
}

void MainWindow::onRescanDone(ZList<KeyboardDevice> list){
    scanning = false;
    ui->rescanButton->setEnabled(true);
    ui->progressBar->setValue(100);
    ui->progressBar->setMaximum(100);
    ui->statusBar->showMessage("Scan Done");
    if(list.size()){
        QStringList slist;
        for(auto it = list.begin(); it.more(); ++it){
            slist.push_back(toQStr(it.get().name));
        }
        ui->keyboardSelect->addItems(slist);
        ui->keyboardSelect->setEnabled(true);
    } else {

    }
}
