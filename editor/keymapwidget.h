#ifndef KEYMAPWIDGET_H
#define KEYMAPWIDGET_H

#include <QWidget>
#include <QQuickWidget>

#include "zarray.h"
#include "zlist.h"
#include "zstring.h"
using namespace LibChaos;

struct KeymapConfig {
    QList<int> layout;
    ZArray<QList<QString>> layers;
};

class KeymapWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeymapWidget(QWidget *parent = nullptr);

    void loadKeymap(KeymapConfig kc);
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
    KeymapConfig kmap;
};

#endif // KEYMAPWIDGET_H
