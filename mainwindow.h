#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainworker.h"

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
    void connectWorker(MainWorker *worker);
    Q_INVOKABLE void customizeKey(int index);

protected:
    // QWidget interface
    void resizeEvent(QResizeEvent *event);

private:
    void startCommand(KeyboardCommand cmd);
    void updateKeyLayout(ZPointer<Keymap> keymap);
    void updateKeyLayer(int index);

signals:
    void doRescan();
    void kbCommand(zu64 key, KeyboardCommand cmd);

public slots:
    void onRescanDone(ZArray<KeyboardDevice> list);
    void onCommandDone(bool ret);

private slots:
    void on_rescanButton_clicked();
    void on_keyboardSelect_currentIndexChanged(int index);
    void on_browseButton_clicked();
    void on_uploadButton_clicked();
    void on_rebootButton_clicked();
    void on_bootButton_clicked();
    void on_fileEdit_textChanged(const QString &arg1);
    void on_layerSelection_currentIndexChanged(int index);
    void updateRepr(int index, QString value);

private:
    Ui::MainWindow *ui;
    bool scanning;
    ZArray<KeyboardDevice> klist;
    KeyboardCommand currcmd;
    QSettings settings;
    ZPointer<Keymap> currentkeymap;

    const QString CUSTOM_FIRMWARE_LOCATION = "customFirmwareLocation";
};

#endif // MAINWINDOW_H
