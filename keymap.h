#ifndef KEYMAP_H
#define KEYMAP_H

#include <QWidget>
#include <QQuickWidget>

class KeyMap : public QWidget
{
    Q_OBJECT
public:
    explicit KeyMap(QWidget *parent = nullptr);
    Q_INVOKABLE QList<int> getKeymap();

signals:

public slots:

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QQuickWidget *view;
};

#endif // KEYMAP_H
