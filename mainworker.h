#ifndef MAINWORKER_H
#define MAINWORKER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "zstring.h"
#include "zmap.h"
#include "zqueue.h"
#include "zfile.h"
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
    CMD_KM_SET,     //! Set keymap.
    CMD_KM_COMMIT,  //! Commit keymap to flash.
    CMD_KM_RELOAD,  //! Reset to keymap stored in flash.
    CMD_KM_RESET,   //! Reset to hardcoded default keymap.
    CMD_FLASH,      //! Flash firmware to keyboard.
};

struct FirmwareDesc {
    ZString name;
    ZString file;
};

struct KeyboardDevice {
    DeviceType devtype;
    ZString name;
    ZString slug;
    ZString version;
    ZString fw_str;
    zu64 key;
    int flags;
    ZPointer<Keymap> keymap;
    ZArray<FirmwareDesc> updates;
};

struct KeyboardFirmware {
    ZString file;
    ZBinary md5;
    bool dl_ok;
    bool fw_ok;
    ZString name;
    ZString slug;
    ZString version;
};

class MainWorker : public QObject {
    Q_OBJECT
public:
    explicit MainWorker(bool fake, QObject *parent = nullptr);

private:
    void downloadFile(ZString url, ZString name);
    void startDownload(QUrl url);
    void checkFirmwareFile(ZFile *file, KeyboardFirmware *fw);

signals:
    void rescanDone(ZArray<KeyboardDevice> list);
    void commandDone(KeyboardCommand cmd, bool ret);
    void keymapUpdate(zu64 key, ZPointer<Keymap> keymap);
    void statusUpdate(ZString status);
    void progressUpdate(int val, int max);
    void checkedForUpdate();

public slots:
    void onStartup();
    void onDoRescan();
    void onKbCommand(zu64 key, KeyboardCommand cmd, QVariant arg1, QVariant arg2);
    void onKbKmUpdate(zu64 key, ZPointer<Keymap> keymap);
    void onRefreshKeymap(zu64 key);

private slots:
    void downloadFinished();
    void downloadReadyRead();

private:
    ZMap<zu64, KBDevice> kdevs;
    bool fake;
    QNetworkAccessManager *netmgr;
    QNetworkReply *reply;
    ZPath app_dir;
    ZFile dlfile;
    ZArray<KeyboardFirmware> kb_fw;
    int fw_i;
    ZHash<ZBinary, ZHashBase::MD5> *dl_hash;
    ZMap<DeviceType, int> known_devices;
};

#endif // MAINWORKER_H
