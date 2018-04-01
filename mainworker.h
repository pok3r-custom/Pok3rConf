#ifndef MAINWORKER_H
#define MAINWORKER_H

#include <QObject>

class MainWorker : public QObject {
    Q_OBJECT
public:
    explicit MainWorker(QObject *parent = nullptr);

signals:
    void updateList(QList<QString> list);

public slots:
    void doRescan();
};

#endif // MAINWORKER_H
