#include <QCoreApplication>

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

    ui->version->setText("Version: " + QCoreApplication::applicationVersion());
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::connectWorker(MainWorker *worker){
    connect(this, SIGNAL(doRescan()), worker, SLOT(onDoRescan()));
    connect(worker, SIGNAL(rescanDone(ZArray<KeyboardDevice>)), this, SLOT(onRescanDone(ZArray<KeyboardDevice>)));
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
        ui->flashButton->setEnabled(false);

        ui->tabWidget->insertTab(0, ui->noTab, "No Keyboard");
        for(int i = 1; i < ui->tabWidget->count(); ++i)
            ui->tabWidget->setTabEnabled(i, false);
    }
}

void MainWindow::on_keyboardSelect_currentIndexChanged(int index){
    if(index >= 0){
        ui->keyboardName->setText(toQStr("Keyboard name: " + klist[index].name));
        ui->firmwareVersion->setText(toQStr("Keyboard version: " + klist[index].version));
        if(klist[index].flags & FLAG_SUPPORTED){
            ui->supported->setText("This keyboard is supported");
            ui->supported->setStyleSheet("QLabel { color: green; }");
            ui->flashButton->setEnabled(true);
        } else {
            ui->supported->setText("Keyboard is not yet supported");
            ui->supported->setStyleSheet("QLabel { color: red; }");
            ui->flashButton->setEnabled(false);
        }
    }
}

void MainWindow::onRescanDone(ZArray<KeyboardDevice> list){
    scanning = false;
    klist = list;
    ui->rescanButton->setEnabled(true);
    ui->progressBar->setValue(100);
    ui->progressBar->setMaximum(100);
    if(list.size()){
        ui->statusBar->showMessage("Scan Done");
        ui->tabWidget->removeTab(0);
        for(int i = 0; i < ui->tabWidget->count(); ++i)
            ui->tabWidget->setTabEnabled(i, true);

        QStringList slist;
        for(auto it = list.begin(); it.more(); ++it){
            slist.push_back(toQStr(it.get().name));
        }
        ui->keyboardSelect->addItems(slist);
        ui->keyboardSelect->setEnabled(true);
    } else {
        ui->statusBar->showMessage("No Keyboards Detected");
    }
}
