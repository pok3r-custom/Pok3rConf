#include "keymap.h"

#include <QDateTime>
#include <QQmlContext>
#include <QLayout>
#include "zlog.h"
#include "zlist.h"

using namespace LibChaos;

const ZList<ZList<int>> layoutAnsi60 = {
    { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, },
    { 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, },
    { 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 9, },
    { 9, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 11, },
    { 5, 5, 5, 25, 5, 5, 5, 5, },
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

QList<int> KeyMap::getKeymap()
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

void KeyMap::resizeEvent(QResizeEvent *event)
{
    QObject *obj = (QObject*) view->rootObject();
    QMetaObject::invokeMethod(obj, "setSize",
                              Q_ARG(QVariant, event->size().width()),
                              Q_ARG(QVariant, event->size().height()));
}

void KeyMap::paintEvent(QPaintEvent *event)
{
}

