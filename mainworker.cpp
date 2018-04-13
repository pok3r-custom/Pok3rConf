#include "mainworker.h"

#include "pok3rtool/proto_pok3r.h"
#include "pok3rtool/proto_cykb.h"
#include "pok3rtool/rawhid/hiddevice.h"
#include "pok3rtool/kbproto.h"
#include "pok3rtool/kbscan.h"

#include "zlog.h"
#include "zmap.h"
#include "zrandom.h"

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
        zu64 key = random.genzu();
        kdevs.add(key, dev);
        list.push({ dev.info.name, version, FLAG_NONE, key });
    }

    if(fake){
        list.push({ "Fake Pok3r", "N/A", FLAG_NONE, 0 });
    }

    for(auto it = list.begin(); it.more(); ++it){
        auto dev = it.get();
        LOG(dev.name << ": " << dev.version << " [" << dev.key << "]");
    }

    LOG("<< Rescan Done");
    emit rescanDone(list);
}

void MainWorker::onKbCommand(zu64 key, KeyboardCommand cmd){
    if(!kdevs.contains(key)){
        emit commandDone(false);
        return;
    }
    auto dev = kdevs[key];
    bool ret = false;

    switch(cmd){
        case CMD_REBOOT:
            ret = dev.iface->rebootFirmware(true);
            break;
        case CMD_BOOTLOADER:
            ret = dev.iface->rebootBootloader(true);
            break;
        default:
            break;
    }

    emit commandDone(ret);
}
