#include "mainworker.h"
#include <QVariant>

#include "pok3rtool/proto_pok3r.h"
#include "pok3rtool/proto_cykb.h"
#include "pok3rtool/rawhid/hiddevice.h"
#include "pok3rtool/kbproto.h"
#include "pok3rtool/kbscan.h"

#include "gen_firmware.h"

#include "zlog.h"
#include "zmap.h"
#include "zrandom.h"

const ZMap<DeviceType, int> known_devices = {
    { DEV_POK3R,        FLAG_SUPPORTED },
    { DEV_POK3R_RGB,    FLAG_NONE },
    { DEV_POK3R_RGB2,   FLAG_NONE },
    { DEV_KBP_V60,      FLAG_NONE },
    { DEV_KBP_V80,      FLAG_NONE },
};

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

MainWorker::MainWorker(bool f, QObject *parent) : QObject(parent), fake(f){

}

void MainWorker::onDoRescan(){
    LOG(">> Start Rescan");

    kdevs.clear();

    KBScan scanner;
    ZRandom random;
    ZArray<KeyboardDevice> list;

    scanner.scan();
    auto devs = scanner.open();
    for(auto it = devs.begin(); it.more(); ++it){
        auto dev = it.get();
        ZString version = dev.iface->getVersion();
        int flags = FLAG_NONE;

        if(dev.iface->isBuiltin()){
            flags |= FLAG_BOOTLOADER;
        }

        ZPointer<Keymap> km;
        if(dev.iface->isQMK()){
            ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
            km = qmk->loadKeymap();
            if(!km.get()){
                ELOG("Failed to load keymap");
            }
            flags |= FLAG_QMK;
        }

        if(known_devices.contains(dev.devtype)){
            flags |= known_devices[dev.devtype];
        }

        zu64 key = random.genzu();
        kdevs.add(key, dev);
        list.push({ dev.devtype, dev.info.name, version, key, flags, km });
    }

    if(fake){
        list.push({ DEV_POK3R, "Fake Pok3r", "N/A", 0, FLAG_NONE, nullptr });
    }

    for(auto it = list.begin(); it.more(); ++it){
        auto dev = it.get();
        ArZ flags;
        if(dev.flags & FLAG_BOOTLOADER) flags.push("BOOT");
        if(dev.flags & FLAG_QMK) flags.push("QMK");
        if(dev.flags & FLAG_SUPPORTED) flags.push("SUPPORT");
        LOG(dev.name << ": " << dev.version <<
            (flags.size() ? " (" + ZString::join(flags, ",") + ")" : "") <<
            " [" << dev.key << "]");
    }

    LOG("<< Rescan Done");
    emit rescanDone(list);
}

void MainWorker::onKbCommand(zu64 key, KeyboardCommand cmd, QVariant arg1, QVariant arg2){
    if(!kdevs.contains(key)){
        ELOG("Command for bad keyboard");
        emit commandDone(false);
        return;
    }
    auto dev = kdevs[key];
    bool ret = false;

    LOG("Command " << dev.info.name << ": " << cmd << " " << arg1.toString().toStdString() << " " << arg2.toString().toStdString());

    switch(cmd){
        case CMD_REBOOT:
            ret = dev.iface->rebootFirmware(true);
            break;
        case CMD_BOOTLOADER:
            ret = dev.iface->rebootBootloader(true);
            break;
        case CMD_KM_COMMIT: {
            zassert(dev.iface->isQMK(), "kb not qmk");
            ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
            //ret = qmk->commitKeymap();
            break;
        }
        default:
            break;
    }

    emit commandDone(ret);
}

void MainWorker::onKbKmUpdate(zu64 key, ZPointer<Keymap> keymap){
    if(!kdevs.contains(key)){
        ELOG("Command for bad keyboard");
        emit commandDone(false);
        return;
    }
    auto dev = kdevs[key];

    zassert(dev.iface->isQMK(), "kb not qmk");
    ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
    bool ret = qmk->uploadKeymap(keymap);
    emit commandDone(ret);
}
