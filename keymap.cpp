#include "keymap.h"
#include "keycustomize.h"

#include <QDateTime>
#include <QQmlContext>
#include <QLayout>

#include "zlog.h"
#include "zlist.h"
#include "zjson.h"

using namespace LibChaos;

KeyMap::KeyMap(QWidget *parent) : QWidget(parent)
{
    view = new QQuickWidget();
    view->rootContext()->setContextProperty("keymapper", this);
    view->setSource(QUrl(QStringLiteral("qrc:///keymap.qml")));
    view->setClearColor(Qt::transparent);
    // Needed to make the background transparent
    view->setAttribute(Qt::WA_AlwaysStackOnTop, true);
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

    try {
        ZJSON json;
        if(!json.decode(data.toStdString())){
            LOG("JSON parse error");
            return;
        }

        LOG("Layout: " << json["name"].string());
        int keycount = 0;
        for(auto it = json["layout"].array().begin(); it.more(); ++it){
            ZList<int> row;
            for(auto jt = it.get().array().begin(); jt.more(); ++jt){
                int width = jt.get().number();
                row.push(width);
                if(width < 100)
                    ++keycount;
            }
            layout.push(row);
        }

        for(auto it = json["layers"].array().begin(); it.more(); ++it){
            ZList<QString> layer;
            for(auto jt = it.get().array().begin(); jt.more(); ++jt){
                layer.push(jt.get().string().cc());
            }
            if(layer.size() != keycount)
                throw ZException(ZString("Invalid layer keymap size: ") + layer.size());
            layers.push(layer);
        }

        // print out layout
        /*int i = 0;
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
                RLOG(" '" << jt.get().toStdString() << "'");
            }
            RLOG(ZLog::NEWLN);
        }*/

        LOG("EMIT " << layers.size());
        emit keymapLoaded(layers.size());

        setLayer(0);
    } catch(ZException e){
        ELOG("Keymap error: " << e.what());
    }
}

void KeyMap::setLayer(int layer)
{
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "updateLayout",
                              Q_ARG(QVariant, layer));
}

QList<int> KeyMap::getKeyLayout()
{
    QList<int> keymap;
    for (auto it = layout.cbegin(); it.more(); ++it) {
        for (auto jt = it.get().cbegin(); jt.more(); ++jt) {
            keymap.append(jt.get());
        }
        keymap.append(-1);
    }
    return keymap;
}

QList<QString> KeyMap::getKeyLayer(int layer)
{
    QList<QString> keymap;
    for (auto it = layers[layer].cbegin(); it.more(); ++it) {
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
