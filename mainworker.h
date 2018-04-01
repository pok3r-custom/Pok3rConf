#ifndef MAINWORKER_H
#define MAINWORKER_H

#include <QObject>

class MainWorker : public QObject {
    Q_OBJECT
public:
    explicit MainWorker(QObject *parent = nullptr);

signals:
    void rescanDone(QStringList list);

public slots:
    void onDoRescan();
};

#endif // MAINWORKER_H
