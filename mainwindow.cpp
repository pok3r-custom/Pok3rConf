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

enum {
    TAB_NO_KB = 0,
    TAB_FIRMWARE,
    TAB_KEYMAP,
    TAB_COMMANDS,
    TAB_ABOUT,
};

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
    keyCustomize = new KeyCustomize(this);
    connect(keyCustomize, SIGNAL(acceptedKey(int,QString)), this, SLOT(updateRepr(int,QString)));

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
    ui->tabWidget->addTab(tabs[TAB_NO_KB], tabnames[TAB_NO_KB]);
    ui->tabWidget->addTab(tabs[TAB_FIRMWARE], tabnames[TAB_FIRMWARE]);
    ui->tabWidget->addTab(tabs[TAB_KEYMAP], tabnames[TAB_KEYMAP]);
    if(devel){
        // Show commands tab in developer mode
        ui->tabWidget->addTab(tabs[TAB_COMMANDS], tabnames[TAB_COMMANDS]);
    }
    ui->tabWidget->addTab(tabs[TAB_ABOUT], tabnames[TAB_ABOUT]);


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

// Command start/done

void MainWindow::startCommand(KeyboardCommand cmd, QVariant arg1, QVariant arg2){
    int index = ui->keyboardSelect->currentIndex();
    if(index >= 0 && currcmd == CMD_NONE){
        currcmd = cmd;
        ui->progressBar->setMaximum(0);
        ui->rebootButton->setEnabled(false);
        ui->bootButton->setEnabled(false);

        switch(cmd){
            case CMD_KM_SET: {
                ui->statusBar->showMessage("Update Keymap...");
                // make copy of keymap for worker thread
                ZPointer<Keymap> km = new Keymap(*currentkeymap.get());
                emit kbKmUpdate(klist[index].key, km);
                break;
            }
            default:
                ui->statusBar->showMessage("Command...");
                emit kbCommand(klist[index].key, cmd, arg1, arg2);
                break;
        }
    }
}

void MainWindow::onCommandDone(KeyboardCommand cmd, bool ret){
    currcmd = CMD_NONE;

    ui->progressBar->setValue(100);
    ui->progressBar->setMaximum(100);
    ui->rebootButton->setEnabled(true);
    ui->bootButton->setEnabled(true);
    if(ret){
        ui->statusBar->showMessage("Done");
    }  else {
        ui->statusBar->showMessage("Error");
    }

    if(cmd == CMD_REBOOT || cmd == CMD_BOOTLOADER){
        on_rescanButton_clicked();
    } else if(cmd == CMD_KM_RELOAD || cmd == CMD_KM_RESET){

    }
}

void MainWindow::onKeymapUpdate(zu64 key, ZPointer<Keymap> keymap){
    if(kmap.contains(key)){
        LOG("Update Keymap " << kmap[key]->slug);
        kmap[key]->keymap = keymap;
        updateKeyLayout(kmap[key]->keymap);
    }
}

void MainWindow::onStatusUpdate(ZString status){
    ui->statusBar->showMessage(toQStr(status));
}

void MainWindow::onProgressUpdate(int val, int max){
    ui->progressBar->setValue(val);
    if(max != -1)
        ui->progressBar->setMaximum(max);
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

        ui->tabWidget->insertTab(TAB_NO_KB, ui->noTab, "No Keyboard");
        ui->tabWidget->setCurrentIndex(TAB_NO_KB);
        for(int i = 1; i < ui->tabWidget->count() - 1; ++i)
            ui->tabWidget->setTabEnabled(i, false);
    }
}

void MainWindow::onRescanDone(ZArray<KeyboardDevice> list){
    klist = list;
    scanning = false;
    ui->rescanButton->setEnabled(true);
    ui->progressBar->setValue(100);
    ui->progressBar->setMaximum(100);
    if(klist.size()){
        ui->statusBar->showMessage("Scan Done");
        ui->tabWidget->removeTab(TAB_NO_KB);
        for(int i = 0; i < ui->tabWidget->count(); ++i)
            ui->tabWidget->setTabEnabled(i, true);

        QStringList slist;
        for(auto it = klist.begin(); it.more(); ++it){
            kmap[it.get().key] = &it.get();

            ZString kbname = it.get().name + " (" + it.get().version + ")";
            slist.push_back(toQStr(kbname));
        }
        ui->keyboardSelect->addItems(slist);
        ui->keyboardSelect->setEnabled(true);
    } else {
        ui->statusBar->showMessage("No Keyboards Detected");
    }
}

void MainWindow::on_keyboardSelect_currentIndexChanged(int index){
    // Change selected keyboard
    if(index >= 0){
        ui->keyboardName->setText(toQStr("Keyboard: " + klist[index].name));
        ui->firmwareVersion->setText(toQStr("Firmware: " + klist[index].fw_str));

        // Add available firmware to list
        ui->firmwareSelect->clear();
        if(klist[index].updates.size()){
            ui->supported->setText("Firmware update available");
            ui->supported->setStyleSheet("QLabel { color: green; }");

            ui->firmwareSelect->setEnabled(true);
            for(zsize i = 0; i < klist[index].updates.size(); ++i){
                ui->firmwareSelect->addItem(toQStr(klist[index].updates[i].name));
            }
            ui->flashButton->setEnabled(true);
        } else {
            ui->supported->setText("No firmware available");
            ui->supported->setStyleSheet("QLabel { color: red; }");

            ui->firmwareSelect->setEnabled(false);
            ui->firmwareSelect->addItem("No Firmware Available");
            ui->flashButton->setEnabled(false);
        }

        // Update keymap for keyboard
        if(klist[index].keymap.get()){
            ui->tabWidget->setTabEnabled(TAB_KEYMAP-1, true);
            //LOG("Layout: " << klist[index].keymap->layoutName());
            updateKeyLayout(klist[index].keymap);
        } else {
            ui->tabWidget->setTabEnabled(TAB_KEYMAP-1, false);
        }
    }
}

void MainWindow::on_flashButton_clicked(){
    int kbi = ui->keyboardSelect->currentIndex();
    int fwi = ui->firmwareSelect->currentIndex();
    //LOG("Flash: " << klist[kbi].updates[fwi].name << " -> " << klist[kbi].slug);
    startCommand(CMD_FLASH, toQStr(klist[kbi].updates[fwi].file));
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

void MainWindow::on_commitButton_clicked(){
    startCommand(CMD_KM_COMMIT);
}

void MainWindow::on_resetButton_clicked(){
    startCommand(CMD_KM_RELOAD);
}

void MainWindow::on_defaultButton_clicked(){
    startCommand(CMD_KM_RESET);
}

void MainWindow::on_fileEdit_textChanged(const QString &arg1){
    settings.setValue(CUSTOM_FIRMWARE_LOCATION, arg1);
}

void MainWindow::on_layerSelection_currentIndexChanged(int index){
    currentLayer = index;
    // Change selected layer
    if(index >= 0)
        updateKeyLayer(index);
}

void MainWindow::customizeKey(int index){
    keyCustomize->setKey(index, toQStr(currentkeymap->keycodeName(currentkeymap->get(currentLayer, index))));
    keyCustomize->show();
    //keyCustomize->accepted();
}

void MainWindow::updateRepr(int index, QString value){
    ZString key = value.toStdString();
    LOG("Map key: " << index << " -> " << key);

    Keymap::keycode kc = currentkeymap->toKeycode(key);
    ZString abbr = currentkeymap->keycodeAbbrev(kc);

    QObject *obj = (QObject *)ui->keymap->rootObject();
    QMetaObject::invokeMethod(obj, "updateRepr", Q_ARG(QVariant, index), Q_ARG(QVariant, toQStr(abbr)));

    zassert(currentLayer >= 0 && currentLayer < currentkeymap->numLayers(), "bad key layer");
    zassert(index >= 0 && index < currentkeymap->numKeys(), "bad key index");

    currentkeymap->set(currentLayer, index, kc);
    startCommand(CMD_KM_SET);
}

void MainWindow::updateKeyLayout(ZPointer<Keymap> keymap){
    LOG("updateLayout " << keymap->layoutName());
    currentkeymap = keymap;

    QVariantList list;
    ZArray<int> layout = currentkeymap->getLayout();
    for(auto it = layout.begin(); it.more(); ++it){
        list << it.get();
    }

    QObject *obj = (QObject *)ui->keymap->rootObject();
    QMetaObject::invokeMethod(obj, "setKeyLayout", Q_ARG(QVariant, list));

    ui->layerSelection->clear();
    for(int i = 0; i < currentkeymap->numLayers(); ++i){
        ui->layerSelection->addItem(QString("Layer %1").arg(i));
    }
}

void MainWindow::updateKeyLayer(int index){
    LOG("updateLayer " << index);

    QVariantList list;
    //ZArray<int> layer = currentkeymap->getLayer(index);
    ZArray<ZString> layer = currentkeymap->getLayerAbbrev(index);
    for(auto it = layer.begin(); it.more(); ++it){
        list << toQStr(it.get());
    }

    QObject *obj = (QObject*) ui->keymap->rootObject();
    QMetaObject::invokeMethod(obj, "updateLayer", Q_ARG(QVariant, list));
}
