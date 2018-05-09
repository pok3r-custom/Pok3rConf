#include <QCoreApplication>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QPushButton>
#include <QGridLayout>
#include <QResizeEvent>
#include <QQuickView>
#include <QQuickWidget>
#include <QDateTime>
#include <QQmlContext>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editor/keycustomize.h"

#include "zjson.h"
#include "zlog.h"

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

MainWindow::MainWindow(bool devel, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scanning(false),
    currcmd(CMD_NONE)
{
    ui->setupUi(this);

    // Store and remove tabs
    QList<QWidget*> tabs;
    QList<QString> tabnames;
    int count = ui->tabWidget->count();
    for(int i = count; i > 0; --i){
        tabs.push_front(ui->tabWidget->widget(i-1));
        tabnames.push_front(ui->tabWidget->tabText(i-1));
        ui->tabWidget->removeTab(i-1);
    }

    // Re-add tabs
    ui->tabWidget->addTab(tabs[0], tabnames[0]);
    ui->tabWidget->addTab(tabs[1], tabnames[1]);
    ui->tabWidget->addTab(tabs[2], tabnames[2]);
    if(devel){
        // Show commands tab in developer mode
        ui->tabWidget->addTab(tabs[3], tabnames[3]);
    }
    ui->tabWidget->addTab(tabs[4], tabnames[4]);


    ui->version->setText("Version: " + QCoreApplication::applicationVersion());
    ui->tabWidget->setCurrentIndex(0);
    ui->fileEdit->setText(settings.value(CUSTOM_FIRMWARE_LOCATION).toString());

    // Set keymap qml properties
    ui->keymap->rootContext()->setContextProperty("mainwindow", this);
    ui->keymap->setClearColor(Qt::transparent);
    // Needed to make the background transparent
    ui->keymap->setAttribute(Qt::WA_AlwaysStackOnTop, true);
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

void MainWindow::resizeEvent(QResizeEvent *event){
//    auto list = ui->keymap->findChildren<KeymapButton *>();
//    int width = event->size().width();
//    int height = event->size().height();
//    int width = ui->keymapContainer->size().width();
//    int height = ui->keymapContainer->size().height();
//    LOG("resize " << width << ", " << height);
//    foreach(KeymapButton *w, list){
//        w->forSize(width, height);
//    }
}

// Command start/done

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

    KeyboardCommand cmd = currcmd;
    currcmd = CMD_NONE;
    if(cmd == CMD_REBOOT || cmd == CMD_BOOTLOADER){
        on_rescanButton_clicked();
    }
}

// Rescan start/done

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
        if(klist[index].keymap.get()){
            LOG("Layout: " << klist[index].keymap->layoutName());
            updateKeyLayout(klist[index].keymap);
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

void MainWindow::on_layerSelection_currentIndexChanged(int index)
{
    updateKeyLayer(index);
}

void MainWindow::customizeKey(int index){
   KeyCustomize *keyCustomize = new KeyCustomize(this);
   keyCustomize->show();
   keyCustomize->accepted();
   connect(keyCustomize, &KeyCustomize::acceptedKey, [=](const QString &value){ this->updateRepr(index, value); });
}

void MainWindow::updateRepr(int index, QString value){
    LOG("Map key: " << index << " -> " << value.toStdString());
    QObject *obj = (QObject*) ui->keymap->rootObject();
    QMetaObject::invokeMethod(obj, "updateRepr",
                              Q_ARG(QVariant, index),
                              Q_ARG(QVariant, value));
}

void MainWindow::updateKeyLayout(ZPointer<Keymap> keymap){
    LOG("updateLayout " << keymap->layoutName());
    currentkeymap = keymap;

    QVariantList list;
    ZArray<int> layout = currentkeymap->getLayout();
    for (auto it = layout.begin(); it.more(); ++it) {
        list << it.get();
    }

    QObject *obj = (QObject*) ui->keymap->rootObject();
    QMetaObject::invokeMethod(obj, "setKeyLayout", Q_ARG(QVariant, list));
    updateKeyLayer(0);

    ui->layerSelection->clear();
    for (int i = 0; i < currentkeymap->numLayers(); ++i) {
        ui->layerSelection->addItem(QString("Layer %1").arg(i));
    }
}

void MainWindow::updateKeyLayer(int index){
    LOG("updateLayer " << index);

    QVariantList list;
    //ZArray<int> layer = currentkeymap->getLayer(index);
    ZArray<ZString> layer = currentkeymap->getLayerAbbrev(index);
    for (auto it = layer.begin(); it.more(); ++it) {
        list << toQStr(it.get());
    }

    QObject *obj = (QObject*) ui->keymap->rootObject();
    QMetaObject::invokeMethod(obj, "updateLayer", Q_ARG(QVariant, list));
}
