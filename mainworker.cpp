#include "mainworker.h"
#include <QVariant>
#include <QNetworkRequest>
#include <QUrl>
#include <QStandardPaths>

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
#include "zpath.h"

#define FW_FETCH_URL    "https://gitlab.com/pok3r-custom/qmk_pok3r/-/jobs/artifacts/master/raw/"
//#define FW_FETCH_URL    "https://gitlab.com/pok3r-custom/qmk_pok3r/-/jobs/artifacts/releases/raw/"
#define FW_FETCH_SUFFIX "?job=qmk_pok3r"
#define FW_SUMS_FILE    "qmk_pok3r.md5"

#define FW_TAGS_URL     "https://gitlab.com/api/v4/projects/6944784/repository/tags"

#define FW_INFO_OFFSET  0x160
#define FW_INFO_SIZE    0x80

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

MainWorker::MainWorker(bool f, QObject *parent) : QObject(parent), fake(f),
    netmgr(nullptr), reply(nullptr), dlfile(nullptr), fw_i(0)
{
    app_dir = ZString(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString());
}

void MainWorker::onStartup(){
    // we need to create this object in the worker thread
    netmgr = new QNetworkAccessManager;
    connect(this, SIGNAL(destroyed()), netmgr, SLOT(deleteLater()));

    // check for latest firmware
    emit statusUpdate("Checking for Firmware Update");
    emit progressUpdate(0, 0);
    downloadFile(FW_FETCH_URL FW_SUMS_FILE FW_FETCH_SUFFIX, FW_SUMS_FILE);
    //downloadFile(FW_TAGS_URL, "tags");
}

void MainWorker::downloadFile(ZString url, ZString fname){
    ZPath file = app_dir + fname;

    DLOG("Download " << url << " -> " << file);

    if(!ZFile::createDirsTo(file)){
        ELOG("Failed to create dirs");
        return;
    }
    if(!dlfile.open(file, ZFile::READWRITE | ZFile::TRUNCATE)){
        ELOG("Failed to open file for writing");
        return;
    }

    startDownload(toQStr(url));
}

void MainWorker::startDownload(QUrl url){
    dl_hash = new ZHash<ZBinary, ZHashBase::MD5>;
    reply = netmgr->get(QNetworkRequest(url));
    connect(this, SIGNAL(destroyed()), reply, SLOT(deleteLater()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

void MainWorker::downloadReadyRead(){
    QByteArray dat = reply->readAll();
    ZBinary data(dat.data(), dat.size());
    dl_hash->feed(data);
    if(dlfile.write(data) != data.size()){
        ELOG("Failed to write");
    }
}

void MainWorker::downloadFinished(){
    if(reply->error()){
        LOG("HTTP error");
        emit statusUpdate("Download Failed");
        emit progressUpdate(0, 100);
        return;
    }

    DLOG("HTTP OK " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() <<
        " " << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().toStdString());

    // Handle redirections
    QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirect.isNull()){
        QUrl url = redirect.toUrl();
        dlfile.rewind();
        dlfile.resizeFile(0);
        delete dl_hash;
        DLOG("Redirect " << url.toString().toStdString());
        startDownload(url);
        return;
    }

    ZPath path = dlfile.path();
    LOG("Downloaded " << path);
    if(path.last() == FW_SUMS_FILE){
        // Sums file
        dlfile.close();
        delete dl_hash;
        ZString sums = ZFile::readString(path);
        ArZ lines = sums.strExplode("\n");
        // Fetch files listed with sums
        for(auto it = lines.begin(); it.more(); ++it){
            ArZ sp = it.get().strip('\r').explode(' ');
            if(sp.size() != 2){
                ELOG("Invalid sums line");
                continue;
            }

            KeyboardFirmware fw;
            fw.file = sp[1];
            fw.md5 = ZBinary::fromHex(sp[0]);
            fw.dl_ok = false;
            fw.fw_ok = false;

            ZFile file;
            if(file.open(app_dir + fw.file)){
                LOG("Cached " << fw.file);
                ZBinary buf;
                ZHash<ZBinary, ZHashBase::MD5> fhash;
                while(file.read(buf, 1024)){
                    fhash.feed(buf);
                }
                fhash.finish();
                if(fhash.hash() == fw.md5){
                    fw.dl_ok = true;
                    checkFirmwareFile(&file, &fw);
                }
            }
            kb_fw.push(fw);
        }
        fw_i = 0;
        emit progressUpdate(fw_i, kb_fw.size());

    } else if(path.getExtension() == ".bin"){
        // Firmware file
        dl_hash->finish();
        ZBinary md5 = dl_hash->hash();
        delete dl_hash;
        if(md5 != kb_fw[fw_i].md5){
            ELOG("Invalid firmware " << md5 << " " << kb_fw[fw_i].md5);
        } else {
            checkFirmwareFile(&dlfile, &(kb_fw[fw_i]));
            dlfile.close();
        }
        emit progressUpdate(++fw_i, -1);
    }

    bool done = true;
    for(int i = fw_i; i < kb_fw.size(); ++i){
        if(!kb_fw[i].dl_ok){
            emit statusUpdate("Downloading " + kb_fw[fw_i].file);
            downloadFile(FW_FETCH_URL + kb_fw[fw_i].file + FW_FETCH_SUFFIX, kb_fw[fw_i].file);
            done = false;
            break;
        }
    }
    if(done){
        emit statusUpdate("Fetched Firmware");
        emit checkedForUpdate();
    }
}

void MainWorker::checkFirmwareFile(ZFile *file, KeyboardFirmware *fw){
    ZBinary info_bin;
    if( (file->seek(FW_INFO_OFFSET) != FW_INFO_OFFSET) || (file->read(info_bin, FW_INFO_SIZE) != FW_INFO_SIZE)){
        ELOG("File read error");
    }
    ZString info_str(info_bin.raw(), info_bin.size());
    ArZ info = info_str.explode(';');
    if(info.size() == 4){
        fw->fw_ok = true;
        fw->name = info[0];
        fw->slug = info[1];
        fw->version = info[2];
        DLOG("Firmware OK: " << fw->file << " " << fw->slug << " " << fw->version);
    } else {
        ELOG("Invalid firmware info");
    }
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
        int flags = FLAG_NONE;
        ZString ver_str = dev.iface->getVersion();
        ZString fw_str;
        ZPointer<Keymap> km;
        ZString version = ver_str;

        if(dev.iface->isBuiltin()){
            flags |= FLAG_BOOTLOADER;
            fw_str = "Stock Bootloader";
        } else if(dev.iface->isQMK()){
            ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
            ZString qmk_ver = qmk->qmkVersion();
            version = qmk_ver;
            km = qmk->loadKeymap();
            if(!km.get()){
                ELOG("Failed to load keymap");
            }
            flags |= FLAG_QMK;
            fw_str = "qmk_pok3r " + qmk_ver;
        } else {
            fw_str = "Vortex Firmware " + ver_str;
        }

        if(known_devices.contains(dev.devtype)){
            flags |= known_devices[dev.devtype];
        }

        zu64 key = random.genzu();
        kdevs.add(key, dev);

        KeyboardDevice kbdev;
        kbdev.devtype = dev.devtype;
        kbdev.name = dev.info.name;
        kbdev.slug = dev.info.slug;
        kbdev.version = ver_str;
        kbdev.fw_str = fw_str;
        kbdev.key = key;
        kbdev.flags = flags;
        kbdev.keymap = km;

        for(auto it = kb_fw.cbegin(); it.more(); ++it){
            if(it.get().fw_ok && it.get().slug == dev.info.slug){
                if(it.get().version != version){
                    //LOG("Firmware Update " << version << " -> " << it.get().version << " for " << dev.info.slug);
                    kbdev.updates.push({
                        it.get().name + " " + it.get().version,
                        it.get().file
                    });
                    break;
                }
            }
        }

        list.push(kbdev);
    }

    if(fake){
        KeyboardDevice kbdev;
        kbdev.devtype = DEV_POK3R;
        kbdev.name = "Fake Pok3r";
        kbdev.slug = "vortex/pok3r";
        kbdev.version = "N/A";
        kbdev.fw_str = "Fake Firmware v0.0";
        kbdev.key = 0;
        kbdev.flags = FLAG_NONE;
        kbdev.keymap = nullptr;
        list.push(kbdev);
    }

    for(auto it = list.begin(); it.more(); ++it){
        auto dev = it.get();
        ArZ flags;
        if(dev.flags & FLAG_BOOTLOADER) flags.push("BOOT");
        if(dev.flags & FLAG_QMK) flags.push("QMK");
        if(dev.flags & FLAG_SUPPORTED) flags.push("SUPPORT");
        LOG(dev.name << " (" << dev.slug << "): " << dev.version <<
            (flags.size() ? " {" + ZString::join(flags, ",") + "}" : "") <<
            " [" << dev.key << "]");
        for(zsize i = 0; i < dev.updates.size(); ++i){
            LOG("  Firmware: " << dev.updates[i].name << " - " << dev.updates[i].file);
        }
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
            onRefreshKeymap(key);
            break;
        }
        case CMD_KM_RESET: {
            zassert(dev.iface->isQMK(), "kb not qmk");
            ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
            ret = qmk->resetKeymap();
            onRefreshKeymap(key);
            break;
        }

        case CMD_FLASH: {

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

void MainWorker::onRefreshKeymap(zu64 key){
    if(kdevs.contains(key)){
        auto dev = kdevs[key];
        zassert(dev.iface->isQMK(), "kb not qmk");
        ProtoQMK *qmk = dynamic_cast<ProtoQMK*>(dev.iface.get());
        auto km = qmk->loadKeymap();
        emit keymapUpdate(key, km);
    }
}
