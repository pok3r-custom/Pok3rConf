#include "mainwindow.h"
#include "mainworker.h"

#include <QApplication>
#include <QThreadPool>

#include "zoptions.h"
#include "zlog.h"

using namespace LibChaos;

#define OPT_VERBOSE "verbose"
#define OPT_FAKE    "fake"
#define OPT_DEVEL   "devel"

const ZArray<ZOptions::OptDef> optdef = {
    { OPT_VERBOSE,  'v', ZOptions::NONE },
    { OPT_FAKE,     0, ZOptions::NONE },    // Add a fake device to device list
    { OPT_DEVEL,    0, ZOptions::NONE },    // Show developer commands and options
};

#define TERM_RESET  "\x1b[m"
#define TERM_RED    "\x1b[31m"
#define TERM_PURPLE "\x1b[35m"

int main(int argc, char *argv[]){
    ZPath lgf = ZPath("logs") + ZLog::genLogFileName("pok3rconf_");
    ZLog::logLevelFile(ZLog::INFO, lgf, "[%time%] N %log%");
    ZLog::logLevelFile(ZLog::DEBUG, lgf, "[%time%] %thread% D [%function%|%file%:%line%] %log%");
    ZLog::logLevelFile(ZLog::ERRORS, lgf, "[%time%] %thread% E [%function%|%file%:%line%] %log%");

    DLOG("Pok3rConf: " _POK3RCONF_DESCRIBE);

    ZOptions options(optdef);
    if(!options.parse(argc, argv))
        return -2;

    ZLog::logLevelStdOut(ZLog::INFO, "[%clock%] N %log%");
    ZLog::logLevelStdErr(ZLog::ERRORS, TERM_RED "[%clock%] E %log%" TERM_RESET);
    if(options.getOpts().contains(OPT_VERBOSE)){
        ZLog::logLevelStdOut(ZLog::DEBUG, TERM_PURPLE "[%clock%] D %log%" TERM_RESET);
    }

    LOG("Starting pok3rconf");

    QApplication app(argc, argv);

    QApplication::setOrganizationName("zennix");
    QApplication::setApplicationName("Pok3rConf");
    QApplication::setApplicationVersion("0.1");

    MainWindow window(options.getOpts().contains(OPT_DEVEL));
    MainWorker worker(options.getOpts().contains(OPT_FAKE));
    QThread thread;

    // connect worker slots
    qRegisterMetaType<zu64>("zu64");
    qRegisterMetaType<ZString>("ZString");
    qRegisterMetaType<ZArray<KeyboardDevice>>("ZArray<KeyboardDevice>");
    qRegisterMetaType<KeyboardCommand>("KeyboardCommand");
    qRegisterMetaType<ZPointer<Keymap>>("ZPointer<Keymap>");
    window.connectWorker(&worker);
    // start scan when thread starts
    QObject::connect(&thread, SIGNAL(started()), &window, SLOT(on_rescanButton_clicked()));
    QObject::connect(&thread, SIGNAL(started()), &worker, SLOT(onStartup()));

    // start thread
    worker.moveToThread(&thread);
    thread.start();

    window.show();
    int ret = app.exec();

    // stop thread
    thread.quit();
    if(!thread.wait(1000)){
        ELOG("Terminating thread");
        thread.terminate();
    }

    return ret;
}
