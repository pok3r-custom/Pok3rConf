#include "keymapbutton.h"

#include "zlog.h"
using namespace LibChaos;

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

KeymapButton::KeymapButton(int id, int u, QWidget *parent) :
        QToolButton(parent), units(u){
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//    QSizePolicy p = sizePolicy();
//    p.setHeightForWidth(true);
//    setSizePolicy(p);
    setText(toQStr(id));
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
}

QSize KeymapButton::sizeHint() const {
    QSize size = QToolButton::sizeHint();
    size.rwidth() = (size.height() * (units / 4));
    size.rheight() = qMin(size.width(), size.height());
    return size;
}

int KeymapButton::heightForWidth(int width) const {
    int height = width / (units / 4);
//    int height = width;
    LOG("w/h " << width << " " << height);
    return height;
}

void KeymapButton::forSize(int width, int height){
//    setMinimumHeight(height / 5);
//    setMinimumWidth((height / 5) * (units / 4));
}
