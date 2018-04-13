#ifndef KEYMAPBUTTON_H
#define KEYMAPBUTTON_H

#include <QToolButton>

class KeymapButton : public QToolButton {
    Q_OBJECT
public:
    explicit KeymapButton(int id, int units, QWidget *parent = 0);

    QSize sizeHint() const override;

private:
    int units;
};

#endif // KEYMAPBUTTON_H
