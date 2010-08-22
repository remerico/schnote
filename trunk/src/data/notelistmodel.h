#ifndef NOTELISTMODEL_H
#define NOTELISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QDateTime>
#include <QRegExp>

#include "notedb.h"
#include "notedata.h"

class NoteListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit NoteListModel(QObject *parent = 0);
    NoteListModel(const NoteIndexList &notesIndices, QObject *parent = 0)
            : QAbstractListModel(parent), noteIndexList(notesIndices) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Note noteData(const QModelIndex &index) const;


private:
    NoteIndexList noteIndexList;

signals:

public slots:
    void appendNote(const Note &note);
    void appendNote(const NoteList &list);
    void insertNote(const Note &note, int row);
    void editNote(const Note &note, int row);
    void search(const QString &search);
    void removeNote(int row);
    void refreshData();

};

#endif // NOTELISTMODEL_H
