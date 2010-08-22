#ifndef NOTEITEMDELEGATE_H
#define NOTEITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>

class NoteItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NoteItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:

};

#endif // NOTEITEMDELEGATE_H
