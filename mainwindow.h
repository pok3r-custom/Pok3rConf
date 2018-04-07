#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainworker.h"

#include <QMainWindow>
#include <QSettings>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void connectWorker(MainWorker *worker);

private:
    void startCommand(KeyboardCommand cmd);

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

private:
    Ui::MainWindow *ui;
    bool scanning;
    ZArray<KeyboardDevice> klist;
    QSettings settings;
    const QString CUSTOM_FIRMWARE_LOCATION = "customFirmwareLocation";
};

#endif // MAINWINDOW_H
