#ifndef KEYCUSTOMIZE_H
#define KEYCUSTOMIZE_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace Ui {
    class KeyCustomize;
}

class KeyCustomize : public QDialog {
    Q_OBJECT

public:
    explicit KeyCustomize(QWidget *parent = 0);
    ~KeyCustomize();
    void setKey(int k){ currentKey = k; }

signals:
    void acceptedKey(int index, QString str);

public slots:
    // QDialog interface
    void accept();
    void reject();

    void on_searchKey_textChanged(const QString &str);

private:
    Ui::KeyCustomize *ui;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxy;
    int currentKey;
};

#endif // KEYCUSTOMIZE_H
