#include "mainworker.h"
#include <QVariant>
#include <QNetworkRequest>
#include <QUrl>

#if !QT_CONFIG(ssl)
    #error "Need QNetwork SSL"
#endif

#include "pok3rtool/proto_pok3r.h"
#include "pok3rtool/proto_cykb.h"
#include "pok3rtool/rawhid/hiddevice.h"
#include "pok3rtool/kbproto.h"
#include "pok3rtool/kbscan.h"

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

void MainWorker::startDownload(QUrl url){
    DLOG("Download " << url.toString().toStdString());
    reply = netmgr->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &MainWorker::downloadFinished);
}

void MainWorker::downloadFinished(){
    if(reply->error()){
        LOG("HTTP error");
    } else {
        LOG("OK " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() <<
            " " << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().toStdString());
        QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(!redirect.isNull()){
            QUrl url = redirect.toUrl();
            DLOG("Redirect " << url.toString().toStdString());
            startDownload(url);
        }
    }
}

void MainWorker::onStartup(){
    netmgr = new QNetworkAccessManager;

    // check for latest qmk firmware
    QUrl url = QString("https://gitlab.com/pok3r-custom/qmk_pok3r/-/jobs/artifacts/master/download?job=qmk_pok3r");
    startDownload(url);
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
        KeyboardDevice kbdev;
        kbdev.devtype = dev.devtype,
        kbdev.name = dev.info.name,
        kbdev.slug = dev.info.slug,
        kbdev.version = version,
        kbdev.key = key,
        kbdev.flags = flags,
        kbdev.keymap = km,
        list.push(kbdev);
    }

    if(fake){
        KeyboardDevice kbdev;
        kbdev.devtype = DEV_POK3R,
        kbdev.name = "Fake Pok3r",
        kbdev.slug = "vortex/pok3r",
        kbdev.version = "N/A",
        kbdev.key = 0,
        kbdev.flags = FLAG_NONE,
        kbdev.keymap = nullptr,
        list.push(kbdev);
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
        emit commandDone(cmd, false);
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
            ret = qmk->commitKeymap();
            break;
        }
        case CMD_KM_RELOAD: {
            zassert(dev.iface->isQMK(), "kb not qmk");
            ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
            ret = qmk->reloadKeymap();
            break;
        }
        case CMD_KM_RESET: {
            zassert(dev.iface->isQMK(), "kb not qmk");
            ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
            ret = qmk->resetKeymap();
            break;
        }
        default:
            break;
    }

    emit commandDone(cmd, ret);
}

void MainWorker::onKbKmUpdate(zu64 key, ZPointer<Keymap> keymap){
    if(!kdevs.contains(key)){
        ELOG("Command for bad keyboard");
        emit commandDone(CMD_KM_SET, false);
        return;
    }
    auto dev = kdevs[key];

    zassert(dev.iface->isQMK(), "kb not qmk");
    ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
    bool ret = qmk->uploadKeymap(keymap);
    emit commandDone(CMD_KM_SET, ret);
}
