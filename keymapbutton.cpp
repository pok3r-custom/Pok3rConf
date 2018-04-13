#include "keymapbutton.h"

KeymapButton::KeymapButton(int id, int u, QWidget *parent) :
        QToolButton(parent), units(u){
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QSize KeymapButton::sizeHint() const{
    QSize size = QToolButton::sizeHint();
    size.rwidth() = (size.height() * (units / 4));
    size.rheight() = qMin(size.width(), size.height());
    return size;
}
