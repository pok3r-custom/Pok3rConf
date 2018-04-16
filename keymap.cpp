#include "keymap.h"
#include "keycustomize.h"

#include <QDateTime>
#include <QQmlContext>
#include <QLayout>

#include "zlog.h"
#include "zlist.h"
#include "zjson.h"

using namespace LibChaos;

const ZList<ZList<int>> layoutAnsi60 = {
    { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, },
    { 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, },
    { 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 9, },
    { 9, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 11, },
    { 5, 5, 5, 25, 5, 5, 5, 5, },
};

const ZList<QString> layoutAnsiRepr60 = {
    "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "BACK",
    "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "|",
    "CAPS", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "ENTER",
    "SHIFT", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "SHIFT",
    "CTRL", "META", "ALT", "SPACE", "ALT", "FN", "MENU", "CTRL",
};

KeyMap::KeyMap(QWidget *parent) : QWidget(parent)
{
    view = new QQuickWidget();
    view->rootContext()->setContextProperty("keymapper", this);
    view->setSource(QUrl(QStringLiteral("qrc:///keymap.qml")));
    QLayout *layout = new QVBoxLayout();
    layout->addWidget(view);
    setLayout(layout);
    view->show();
}

void KeyMap::loadKeymap(QString url)
{
    QFile file(url);
    if(!file.open(QIODevice::ReadOnly)){
        LOG("Resource file error");
        return;
    }
    QString data = file.readAll();
    file.close();
    ZJSON json;
    if(!json.decode(data.toStdString())){
        LOG("JSON parse error");
        return;
    }

    LOG("Layout: " << json["name"].string());
    for(auto it = json["layout"].array().begin(); it.more(); ++it){
        ZList<int> row;
        for(auto jt = it.get().array().begin(); jt.more(); ++jt){
            row.push(jt.get().number());
        }
        layout.push(row);
    }

    for(auto it = json["layers"].array().begin(); it.more(); ++it){
        ZList<ZString> row;
        for(auto jt = it.get().array().begin(); jt.more(); ++jt){
            row.push(jt.get().string());
        }
        layers.push(row);
    }

    // print out layout
    int i = 0;
    for (auto it = layout.cbegin(); it.more(); ++it){
        RLOG("R" << i++);
        for (auto jt = it.get().cbegin(); jt.more(); ++jt){
            RLOG(" " << jt.get());
        }
        RLOG(ZLog::NEWLN);
    }
    i = 0;
    for (auto it = layers.begin(); it.more(); ++it){
        RLOG("L" << i++);
        for (auto jt = it.get().cbegin(); jt.more(); ++jt){
            RLOG(" '" << jt.get() << "'");
        }
        RLOG(ZLog::NEWLN);
    }
}

QList<int> KeyMap::getKeyWidth()
{
    QList<int> keymap;
    for (auto it = layoutAnsi60.cbegin(); it.more(); ++it) {
        for (auto jt = it.get().cbegin(); jt.more(); ++jt) {
            keymap.append(jt.get());
        }
        keymap.append(-1);
    }
    return keymap;
}

QList<QString> KeyMap::getKeyRepr()
{
    QList<QString> keymap;
    for (auto it = layoutAnsiRepr60.cbegin(); it.more(); ++it) {
        keymap.append(it.get());
    }
    return keymap;
}

void KeyMap::customize(int index)
{
   KeyCustomize *keyCustomize = new KeyCustomize(this);
   keyCustomize->show();
   keyCustomize->accepted();
   connect(keyCustomize, &KeyCustomize::acceptedKey, [=](const QString &value){ this->updateRepr(index, value); });
}

void KeyMap::resizeEvent(QResizeEvent *event)
{
    // FIXME somehow make this automatic based on layout, so we don't need to do this manually
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "setSize",
                              Q_ARG(QVariant, event->size().width()),
                              Q_ARG(QVariant, event->size().height()));
}

void KeyMap::updateRepr(int index, QString value)
{
    LOG("Map key: " << index << " -> " << value.toStdString());
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "updateRepr",
                              Q_ARG(QVariant, index),
                              Q_ARG(QVariant, value));
}
