#ifndef KEYMAP_H
#define KEYMAP_H

#include <QWidget>
#include <QQuickWidget>

class KeyMap : public QWidget
{
    Q_OBJECT
public:
    explicit KeyMap(QWidget *parent = nullptr);
    Q_INVOKABLE QList<int> getKeyWidth();
    Q_INVOKABLE QList<QString> getKeyRepr();
    Q_INVOKABLE customize(int index);

signals:

public slots:

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);

private:
    QQuickWidget *view;

private slots:
    void updateRepr(int, QString);
};

#endif // KEYMAP_H
