#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainworker.h"
#include "editor/keycustomize.h"

#include <QMainWindow>
#include <QSettings>

namespace Ui {
    class MainWindow;
}

struct KeymapConfig {
    QList<int> layout;
    ZArray<QList<QString>> layers;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(bool devel, QWidget *parent = 0);
    ~MainWindow();
    Q_INVOKABLE void customizeKey(int index);

private:
    void startCommand(KeyboardCommand cmd, QVariant arg1 = QVariant(), QVariant arg2 = QVariant());
    void updateKeyLayout(ZPointer<Keymap> keymap);
    void updateKeyLayer(int index);

signals:
    void doRescan();
    void kbCommand(zu64 key, KeyboardCommand cmd, QVariant arg1, QVariant arg2);
    void kbKmUpdate(zu64 key, ZPointer<Keymap> keymap);

public slots:
    void onRescanDone(ZArray<KeyboardDevice> list);
    void onCommandDone(KeyboardCommand cmd, bool ret);
    void onKeymapUpdate(zu64 key, ZPointer<Keymap> keymap);
    void onStatusUpdate(ZString status);
    void onProgressUpdate(int val, int max);

private slots:
    void on_rescanButton_clicked();
    void on_keyboardSelect_currentIndexChanged(int index);
    void on_flashButton_clicked();
    void on_browseButton_clicked();
    void on_uploadButton_clicked();
    void on_rebootButton_clicked();
    void on_bootButton_clicked();
    void on_commitButton_clicked();
    void on_resetButton_clicked();
    void on_defaultButton_clicked();
    void on_fileEdit_textChanged(const QString &arg1);
    void on_layerSelection_currentIndexChanged(int index);
    void updateRepr(int index, QString value);

private:
    Ui::MainWindow *ui;
    KeyCustomize *keyCustomize;
    bool scanning;
    ZArray<KeyboardDevice> klist;
    ZMap<zu16, KeyboardDevice*> kmap;
    KeyboardCommand currcmd;
    QSettings settings;
    int currentLayer;
    ZPointer<Keymap> currentkeymap;

    const QString CUSTOM_FIRMWARE_LOCATION = "customFirmwareLocation";
};

#endif // MAINWINDOW_H
