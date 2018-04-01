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

    void connectSlots(MainWorker *worker);

public slots:
    void onUpdateList(QList<QString> list);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
