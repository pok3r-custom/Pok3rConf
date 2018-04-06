#ifndef MAINWORKER_H
#define MAINWORKER_H

#include <QObject>
#include "zstring.h"

using namespace LibChaos;

enum KeyboardFlags {
    FLAG_NONE       = 0,
    FLAG_BOOTLOADER = 1,
    FLAG_QMK        = 2,
    FLAG_SUPPORTED  = 4,
};

struct KeyboardDevice {
    ZString name;
    ZString version;
    int flags;
};

class MainWorker : public QObject {
    Q_OBJECT
public:
    explicit MainWorker(QObject *parent = nullptr);

signals:
    void rescanDone(ZArray<KeyboardDevice> list);

public slots:
    void onDoRescan();
};

#endif // MAINWORKER_H
