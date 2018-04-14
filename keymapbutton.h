#ifndef KEYMAPBUTTON_H
#define KEYMAPBUTTON_H

#include <QToolButton>

class KeymapButton : public QToolButton {
    Q_OBJECT
public:
    explicit KeymapButton(int id, int units, QWidget *parent = 0);

    QSize sizeHint() const override;
    int heightForWidth(int width) const override;
    //int widthForHeight(int height) const override;

    void forSize(int width, int height);

private:
    int units;
};

#endif // KEYMAPBUTTON_H
