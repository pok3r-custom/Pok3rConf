#include <QCoreApplication>
#include <QFileDialog>
#include <QDir>
#include <QPushButton>
#include <QGridLayout>
#include <QResizeEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "keymapbutton.h"

#include "zlog.h"

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

#define SP      0x80
#define SP_MSK  0x7F

const ZArray<ZArray<int>> layoutAnsi60 = {
    { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8 },
    { 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6 },
    { 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 9 },
    { 9, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 11 },
    { 5, 5, 5, 25, 5, 5, 5, 5 }
};

const ZArray<ZArray<int>> layoutAnsi80 = {
    { 4, SP|4, 4, 4, 4, 4, SP|2, 4, 4, 4, 4, SP|2, 4, 4, 4, 4, SP|1, 4, 4, 4 },
    { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, SP|1, 4, 4, 4 },
    { 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, SP|1, 4, 4, 4 },
    { 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 9, SP|13 },
    { 9, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 11, SP|5, 4, SP|4 },
    { 5, 5, 5, 25, 5, 5, 5, 5, SP|1, 4, 4, 4 }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scanning(false),
    currcmd(CMD_NONE)
{
    ui->setupUi(this);

    ui->version->setText("Version: " + QCoreApplication::applicationVersion());
    ui->tabWidget->setCurrentIndex(0);
    ui->fileEdit->setText(settings.value(CUSTOM_FIRMWARE_LOCATION).toString());

    auto keyboardLayout = layoutAnsi80;
    QGridLayout *kLayout = (QGridLayout *)ui->keymap->layout();

    int mcol = 0;
    int k = 1;
    for(int i = 0; i < keyboardLayout.size(); ++i){
        int ccol = 0;
        for(int j = 0; j < keyboardLayout[i].size(); ++j){
            int d = keyboardLayout[i][j];
            int u = d & SP_MSK;
            if(d & SP){
                kLayout->addItem(new QSpacerItem(1, 1), i, ccol, 1, u);
            } else {
                kLayout->addWidget(new KeymapButton(k, u, ui->keymap), i, ccol, 1, u);
                ++k;
            }
            ccol += u;
            mcol = qMax(mcol, ccol);
        }
    }
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::connectWorker(MainWorker *worker){
    connect(this, SIGNAL(doRescan()), worker, SLOT(onDoRescan()));
    connect(worker, SIGNAL(rescanDone(ZArray<KeyboardDevice>)), this, SLOT(onRescanDone(ZArray<KeyboardDevice>)));
    connect(this, SIGNAL(kbCommand(zu64,KeyboardCommand)), worker, SLOT(onKbCommand(zu64,KeyboardCommand)));
    connect(worker, SIGNAL(commandDone(bool)), this, SLOT(onCommandDone(bool)));
}

void MainWindow::startCommand(KeyboardCommand cmd){
    int index = ui->keyboardSelect->currentIndex();
    if(index >= 0 && currcmd == CMD_NONE){
        currcmd = cmd;
        ui->progressBar->setMaximum(0);
        ui->rebootButton->setEnabled(false);
        ui->bootButton->setEnabled(false);
        ui->statusBar->showMessage("Command...");

        LOG("Command " << klist[index].name);
        emit kbCommand(klist[index].key, cmd);
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

void MainWindow::onCommandDone(bool ret){
    ui->progressBar->setValue(100);
    ui->progressBar->setMaximum(100);
    ui->rebootButton->setEnabled(true);
    ui->bootButton->setEnabled(true);
    if(ret){
        ui->statusBar->showMessage("Command Done");
    }  else {
        ui->statusBar->showMessage("Command Error");
    }

//    KeyboardCommand cmd = currcmd;
    currcmd = CMD_NONE;
//    if(cmd == CMD_REBOOT || cmd == CMD_BOOTLOADER){
//        on_rescanButton_clicked();
    //    }
}

void MainWindow::resizeEvent(QResizeEvent *event){
    auto list = ui->keymap->findChildren<KeymapButton *>();
//    int width = event->size().width();
//    int height = event->size().height();
    int width = ui->keymapContainer->size().width();
    int height = ui->keymapContainer->size().height();
//    LOG("resize " << width << ", " << height);
    foreach(KeymapButton *w, list){
        w->forSize(width, height);
    }
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
        ui->tabWidget->setCurrentIndex(0);
        for(int i = 1; i < ui->tabWidget->count() - 1; ++i)
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

void MainWindow::on_browseButton_clicked(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Firmware"), QDir::currentPath(), tr("Binary Files (*.bin)"));
    ui->fileEdit->setText(fileName);
}

void MainWindow::on_uploadButton_clicked(){
    if(!ui->fileEdit->text().isEmpty()){
        ZString file = ui->fileEdit->text().toStdString();
        LOG("Upload: " << file);
    }
}

void MainWindow::on_rebootButton_clicked(){
    startCommand(CMD_REBOOT);
}

void MainWindow::on_bootButton_clicked(){
    startCommand(CMD_BOOTLOADER);
}

void MainWindow::on_fileEdit_textChanged(const QString &arg1)
{
    settings.setValue(CUSTOM_FIRMWARE_LOCATION, arg1);
}
