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
    void setLayer(int layer);

    // Can be called from qml
    Q_INVOKABLE QList<int> getKeyLayout();
    Q_INVOKABLE QList<QString> getKeyLayer(int layer);
    Q_INVOKABLE void customize(int index);

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);

private:
    QQuickWidget *view;

private slots:
    void updateRepr(int index, QString value);

signals:
    void keymapLoaded(int layers);

private:
    ZList<ZList<int>> layout;
    ZArray<ZList<QString>> layers;
};

#endif // KEYMAP_H
