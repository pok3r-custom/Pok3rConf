#include "keymap.h"
#include "keycustomize.h"

#include <QDateTime>
#include <QQmlContext>
#include <QLayout>

#include "zlog.h"
#include "zlist.h"
#include "zjson.h"

using namespace LibChaos;

KeyMap::KeyMap(QWidget *parent) : QWidget(parent){
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

void KeyMap::loadKeymap(KeymapConfig kc){
    kmap = kc;

    LOG("EMIT " << kmap.layers.size());
    emit keymapLoaded(kmap.layers.size());

    setLayer(0);
}

void KeyMap::setLayer(int layer){
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "updateLayout", Q_ARG(QVariant, layer));
}

QList<int> KeyMap::getKeyLayout(){
    return kmap.layout;
}

QList<QString> KeyMap::getKeyLayer(int layer){
    if(layer >= 0 && layer < kmap.layers.size()){
        return kmap.layers[layer];
    } else {
        ELOG("getLayer " << layer);
        return QList<QString>();
    }
}

void KeyMap::customize(int index){
   KeyCustomize *keyCustomize = new KeyCustomize(this);
   keyCustomize->show();
   keyCustomize->accepted();
   connect(keyCustomize, &KeyCustomize::acceptedKey, [=](const QString &value){ this->updateRepr(index, value); });
}

void KeyMap::resizeEvent(QResizeEvent *event){
    // FIXME somehow make this automatic based on layout, so we don't need to do this manually
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "setSize",
                              Q_ARG(QVariant, event->size().width()),
                              Q_ARG(QVariant, event->size().height()));
}

void KeyMap::updateRepr(int index, QString value){
    LOG("Map key: " << index << " -> " << value.toStdString());
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "updateRepr",
                              Q_ARG(QVariant, index),
                              Q_ARG(QVariant, value));
}
