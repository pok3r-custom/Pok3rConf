#ifndef MAINWORKER_H
#define MAINWORKER_H

#include <QObject>

#include "zstring.h"
#include "zmap.h"
using namespace LibChaos;

#include "pok3rtool/kbscan.h"

enum KeyboardFlag {
    FLAG_NONE       = 0,
    FLAG_BOOTLOADER = 1,
    FLAG_QMK        = 2,
    FLAG_SUPPORTED  = 4,
};

enum KeyboardCommand {
    CMD_NONE = 0,
    CMD_ERROR,
    CMD_REBOOT,
    CMD_BOOTLOADER,
};

struct KeyboardDevice {
    ZString name;
    ZString version;
    int flags;
    zu64 key;
};

class MainWorker : public QObject {
    Q_OBJECT
public:
    explicit MainWorker(bool fake, QObject *parent = nullptr);

signals:
    void rescanDone(ZArray<KeyboardDevice> list);
    void commandDone(bool ret);

public slots:
    void onDoRescan();
    void onKbCommand(zu64 key, KeyboardCommand cmd);

private:
    ZMap<zu64, KBDevice> kdevs;
    bool fake;
};

#endif // MAINWORKER_H
