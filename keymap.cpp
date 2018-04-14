#include "keymap.h"

#include <QDateTime>
#include <QQmlContext>
#include <QLayout>
#include "zlog.h"
#include "zlist.h"
#include "keycustomize.h"

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

KeyMap::customize(int index)
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
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "updateRepr",
                              Q_ARG(QVariant, index),
                              Q_ARG(QVariant, value));
}
