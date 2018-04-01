#include "mainworker.h"

#include "pok3rtool/proto_pok3r.h"
#include "pok3rtool/proto_cykb.h"
#include "pok3rtool/rawhid/hiddevice.h"
#include "pok3rtool/updateinterface.h"

#include "zlog.h"
#include "zmap.h"
#include <windows.h>

// Enums
// ////////////////////////////////

enum Device {
    DEV_NONE = 0,
    POK3R,          //!< Vortex POK3R
    POK3R_RGB,      //!< Vortex POK3R RGB
    VORTEX_CORE,    //!< Vortex Core
    VORTEX_RACE3,   //!< Vortex Race 3
    VORTEX_TESTER,  //!< Vortex 22-Key Switch Tester
    VORTEX_VIBE,    //!< Vortex ViBE
    KBP_V60,        //!< KBParadise v60 Mini
    KBP_V80,        //!< KBParadise v80
    TEX_YODA_II,    //!< Tex Yoda II
};

enum DevType {
    PROTO_POK3R,    //!< Used exclusively in the POK3R.
    PROTO_CYKB,     //!< Used in new Vortex keyboards, marked with CYKB on the PCB.
                    //!< POK3R RGB, Vortex CORE, Vortex 22-Key Switch Tester.
};

// Types
// ////////////////////////////////

struct VortexDevice {
    ZString name;
    zu16 vid;
    zu16 pid;
    zu16 boot_pid;
    DevType type;
};

struct ListDevice {
    VortexDevice dev;
    ZPointer<HIDDevice> hid;
    bool boot;
};

// Constants
// ////////////////////////////////

const ZMap<Device, VortexDevice> devices = {
    { POK3R,            { "POK3R",          HOLTEK_VID, POK3R_PID,          POK3R_BOOT_PID,         PROTO_POK3R } },
    { POK3R_RGB,        { "POK3R RGB",      HOLTEK_VID, POK3R_RGB_PID,      POK3R_RGB_BOOT_PID,     PROTO_CYKB } },
    { VORTEX_CORE,      { "Vortex Core",    HOLTEK_VID, VORTEX_CORE_PID,    VORTEX_CORE_BOOT_PID,   PROTO_CYKB } },
    { VORTEX_TESTER,    { "Vortex Tester",  HOLTEK_VID, VORTEX_TESTER_PID,  VORTEX_TESTER_BOOT_PID, PROTO_CYKB } },
    { VORTEX_RACE3,     { "Vortex Race 3",  HOLTEK_VID, VORTEX_RACE3_PID,   VORTEX_RACE3_BOOT_PID,  PROTO_CYKB } },
    { VORTEX_VIBE,      { "Vortex ViBE",    HOLTEK_VID, VORTEX_VIBE_PID,    VORTEX_VIBE_BOOT_PID,   PROTO_CYKB } },
    { KBP_V60,          { "KBP V60",        HOLTEK_VID, KBP_V60_PID,        KBP_V60_BOOT_PID,       PROTO_POK3R } },
    { KBP_V80,          { "KBP V80",        HOLTEK_VID, KBP_V80_PID,        KBP_V80_BOOT_PID,       PROTO_POK3R } },
    { TEX_YODA_II,      { "Tex Yoda II",    HOLTEK_VID, TEX_YODA_II_PID,    TEX_YODA_II_BOOT_PID,   PROTO_CYKB } },
};

// Private Functions
// ////////////////////////////////

static inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

// Class Methods
// ////////////////////////////////

MainWorker::MainWorker(QObject *parent) : QObject(parent){

}

void MainWorker::onDoRescan(){
    LOG(">> Start Rescan");
    ZList<ListDevice> devs;
    QStringList list;

    // Get all connected devices from list
    for(auto it = devices.begin(); it.more(); ++it){
        VortexDevice dev = devices[it.get()];

        auto hdev = HIDDevice::openAll(dev.vid, dev.pid, UPDATE_USAGE_PAGE, UPDATE_USAGE);
        for(zu64 j = 0; j < hdev.size(); ++j)
            devs.push({ dev, hdev[j], false });

        auto hbdev = HIDDevice::openAll(dev.vid, dev.boot_pid, UPDATE_USAGE_PAGE, UPDATE_USAGE);
        for(zu64 j = 0; j < hbdev.size(); ++j)
            devs.push({ dev, hbdev[j], true });
    }

    // Read version from each device
    for(auto it = devs.begin(); it.more(); ++it){
        ListDevice ldev = it.get();
        // Check device
        if(ldev.hid.get() && ldev.hid->isOpen()){
            ZPointer<UpdateInterface> iface;
            // Select protocol
            if(ldev.dev.type == PROTO_POK3R){
                iface = new ProtoPOK3R(ldev.dev.vid, ldev.dev.pid, ldev.dev.boot_pid, ldev.boot, ldev.hid.get());
            } else if(ldev.dev.type == PROTO_CYKB){
                iface = new ProtoCYKB(ldev.dev.vid, ldev.dev.pid, ldev.dev.boot_pid, ldev.boot, ldev.hid.get());
            }

            ldev.hid.divorce();
            ZString item = ldev.dev.name + ": " + iface->getVersion();
            list.push_back(toQStr(item));

            LOG(item);
        } else {
            LOG(ldev.dev.name << " not open");
        }
    }

    LOG("<< Rescan Done");
    emit rescanDone(list);
}
