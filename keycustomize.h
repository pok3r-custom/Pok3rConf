#ifndef KEYCUSTOMIZE_H
#define KEYCUSTOMIZE_H

#include <QDialog>

namespace Ui {
class KeyCustomize;
}

class KeyCustomize : public QDialog
{
    Q_OBJECT

public:
    explicit KeyCustomize(QWidget *parent = 0);
    ~KeyCustomize();

private:
    Ui::KeyCustomize *ui;

Q_SIGNALS:
    acceptedKey(QString);

    // QDialog interface
public slots:
    void accept();
};

#endif // KEYCUSTOMIZE_H
