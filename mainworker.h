#ifndef MAINWORKER_H
#define MAINWORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "zstring.h"
#include "zmap.h"
using namespace LibChaos;

#include "pok3rtool/kbscan.h"
#include "pok3rtool/keymap.h"

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
    CMD_KM_SET,
    CMD_KM_COMMIT,  //! Commit keymap to flash.
    CMD_KM_RELOAD,  //! Reset to keymap stored in flash.
    CMD_KM_RESET,   //! Reset to hardcoded default keymap.
};

struct KeyboardDevice {
    DeviceType devtype;
    ZString name;
    ZString slug;
    ZString fw_str;
    ZString version;
    zu64 key;
    int flags;
    ZPointer<Keymap> keymap;
};

class MainWorker : public QObject {
    Q_OBJECT
public:
    explicit MainWorker(bool fake, QObject *parent = nullptr);

private:
    void startDownload(QUrl url);

signals:
    void rescanDone(ZArray<KeyboardDevice> list);
    void commandDone(KeyboardCommand cmd, bool ret);

public slots:
    void onStartup();
    void onDoRescan();
    void onKbCommand(zu64 key, KeyboardCommand cmd, QVariant arg1, QVariant arg2);
    void onKbKmUpdate(zu64 key, ZPointer<Keymap> keymap);

private slots:
    void downloadFinished();

private:
    ZMap<zu64, KBDevice> kdevs;
    bool fake;
    QNetworkAccessManager *netmgr;
    QNetworkReply *reply;
};

#endif // MAINWORKER_H
