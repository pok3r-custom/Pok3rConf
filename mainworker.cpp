#include "mainworker.h"

#include "pok3rtool/proto_pok3r.h"
#include "pok3rtool/proto_cykb.h"
#include "pok3rtool/rawhid/hiddevice.h"
#include "pok3rtool/updateinterface.h"
#include "pok3rtool/kbscan.h"

#include "zlog.h"
#include "zmap.h"

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

MainWorker::MainWorker(QObject *parent) : QObject(parent){

}

void MainWorker::onDoRescan(){
    ZList<KeyboardDevice> list;
    LOG(">> Start Rescan");

    KBScan scanner;
    scanner.scan();
    auto devs = scanner.open();
    for (auto it = devs.begin(); it.more(); ++it){
        auto dev = it.get();
        ZString version = dev.iface->getVersion();
        LOG(dev.info.name << ": " << version);
        list.push({ dev.info.name, version, FLAG_NONE });
    }

    LOG("<< Rescan Done");
    emit rescanDone(list);
}
