#ifndef KEYMAP_H
#define KEYMAP_H

#include <QWidget>
#include <QQuickWidget>

#include "zarray.h"
#include "zlist.h"
#include "zstring.h"
using namespace LibChaos;

class KeyMap : public QWidget {
    Q_OBJECT
public:
    explicit KeyMap(QWidget *parent = nullptr);

    void loadKeymap(QString url);

    // called from qml
    Q_INVOKABLE QList<int> getKeyWidth();
    Q_INVOKABLE QList<QString> getKeyRepr();
    Q_INVOKABLE void customize(int index);

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);

private:
    QQuickWidget *view;

private slots:
    void updateRepr(int, QString);

private:
    ZList<ZList<int>> layout;
    ZArray<ZList<ZString>> layers;
};

#endif // KEYMAP_H
