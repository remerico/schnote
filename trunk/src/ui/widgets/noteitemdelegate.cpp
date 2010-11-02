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

    // Selected item
    if (option.state & (QStyle::State_Selected) ) {

        // Gradient highlight background
        QLinearGradient bgGradient(0, option.rect.top(), 0, option.rect.top() + option.rect.height());
        QColor hColor = option.palette.highlight().color();
        bgGradient.setColorAt(0.0, hColor);
        bgGradient.setColorAt(1.0, hColor.lighter(80));

        painter->fillRect(option.rect, bgGradient);
        textColor = option.palette.highlightedText().color();
    }
    else {
        textColor = option.palette.text().color();

        // Separator line
        painter->setPen(option.palette.light().color());
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    }

    painter->setPen(textColor);
    painter->setFont(textFont);

    int padding = 5;

    // Draw title
    //textFont.setBold(true);
    textFont.setPointSize(option.font.pointSize() + 1);
    textPos += QPoint(padding, QFontMetrics(textFont).lineSpacing());

    painter->setFont(textFont);
    painter->drawText(textPos, index.data().toString());

    // Draw value
    painter->setPen((textColor.value() >= 128) ? textColor.darker(180) : textColor.lighter(180));

    textFont.setBold(false);
    textFont.setPointSize(option.font.pointSize() - 5);
    textPos += QPoint(0, QFontMetrics(textFont).lineSpacing());

    painter->setFont(textFont);

    painter->drawText(textPos, index.data(Qt::UserRole).toString());

    painter->restore();

#endif

}

QSize NoteItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {

    int padding = 5;
    int height = option.fontMetrics.lineSpacing() * 2;

    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(height + (padding * 2));

    return s;

}
