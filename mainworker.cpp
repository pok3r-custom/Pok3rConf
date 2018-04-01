#include "mainworker.h"
#include "pok3rtool/rawhid/hiddevice.h"
#include "pok3rtool/updateinterface.h"

#include "zlog.h"

MainWorker::MainWorker(QObject *parent) : QObject(parent){

}

void MainWorker::doRescan(){
    LOG("Start Rescan");
}
