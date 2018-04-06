#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainworker.h"

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void connectWorker(MainWorker *worker);

signals:
    void doRescan();

public slots:
    void on_rescanButton_clicked();
    void on_keyboardSelect_currentIndexChanged(int index);
    void onRescanDone(ZArray<KeyboardDevice> list);

private:
    Ui::MainWindow *ui;
    bool scanning;
    ZArray<KeyboardDevice> klist;
};

#endif // MAINWINDOW_H
