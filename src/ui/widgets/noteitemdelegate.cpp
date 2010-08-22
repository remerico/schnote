#include "noteitemdelegate.h"

#include <QtDebug>
#include <QStylePainter>

NoteItemDelegate::NoteItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void NoteItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

#define CUSTOM_PAINT

#ifndef CUSTOM_PAINT

    QItemDelegate::paint(painter, option, index);

#else

    painter->save();

    QColor textColor;
    QPoint textPos = option.rect.topLeft();
    QFont textFont = option.font;

    QStyleOptionFocusRect fRect;


    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        QLinearGradient bgGradient(0, option.rect.top(), 0, option.rect.top() + option.rect.height());
        bgGradient.setColorAt(0.0, QColor(210, 210, 210));
        bgGradient.setColorAt(1.0, QColor(60, 60, 60));

        QColor hColor = option.palette.highlight().color();

        bgGradient.setColorAt(0.0, hColor.lighter(180));
        bgGradient.setColorAt(1.0, hColor);

        //painter->fillRect(option.rect, bgGradient);
        painter->fillRect(option.rect, option.palette.highlight());
        textColor = option.palette.highlightedText().color();
    }
    else {

        //painter->fillRect(option.rect, option.palette.background().color());

        textColor = option.palette.text().color();

        painter->setPen(QColor(240, 240, 240));
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    }

    painter->setPen(textColor);
    painter->setFont(textFont);

    int padding = 3;
    int lineSpacing = option.fontMetrics.lineSpacing();

    textPos += QPoint(padding, lineSpacing);
    textFont.setBold(true);
    painter->setFont(textFont);
    painter->drawText(textPos, index.data().toString());

    textPos += QPoint(0, lineSpacing);
    textFont.setBold(false);
    painter->setFont(textFont);
    painter->drawText(textPos, index.data(Qt::UserRole).toString());

    painter->restore();

#endif

}

QSize NoteItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {

    int padding = 3;
    int height = option.fontMetrics.lineSpacing() * 2;

    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(height + (padding * 2));

    return s;

}
