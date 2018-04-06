#include "mainwindow.h"
#include "mainworker.h"

#include <QApplication>
#include <QThreadPool>

#include "zlog.h"

using namespace LibChaos;

#define TERM_RESET  "\x1b[m"
#define TERM_RED    "\x1b[31m"
#define TERM_PURPLE "\x1b[35m"

int main(int argc, char *argv[]){
    ZLog::logLevelStdOut(ZLog::INFO, "[%clock%] N %log%");
    //ZLog::logLevelStdOut(ZLog::DEBUG, TERM_PURPLE "[%clock%] D %log%" TERM_RESET);
    ZLog::logLevelStdErr(ZLog::ERRORS, TERM_RED "[%clock%] E %log%" TERM_RESET);
    ZPath lgf = ZPath("logs") + ZLog::genLogFileName("pok3rconf_");
    ZLog::logLevelFile(ZLog::INFO, lgf, "[%time%] N %log%");
    ZLog::logLevelFile(ZLog::DEBUG, lgf, "[%time%] %thread% D [%function%|%file%:%line%] %log%");
    ZLog::logLevelFile(ZLog::ERRORS, lgf, "[%time%] %thread% E [%function%|%file%:%line%] %log%");

    LOG("Starting application");

    QApplication app(argc, argv);

    QApplication::setOrganizationName("zennix");
    QApplication::setApplicationName("Pok3rConf");
    QApplication::setApplicationVersion("0.1");

    MainWindow window;
    MainWorker worker;
    QThread thread;

    // connect worker slots
    qRegisterMetaType<ZString>("ZString");
    qRegisterMetaType<ZArray<KeyboardDevice>>("ZArray<KeyboardDevice>");
    window.connectWorker(&worker);
    // start scan when thread starts
    QObject::connect(&thread, SIGNAL(started()), &window, SLOT(on_rescanButton_clicked()));

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
