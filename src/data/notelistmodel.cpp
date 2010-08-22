#include "notelistmodel.h"
#include <QtDebug>

NoteListModel::NoteListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    NoteIndexList list = NoteDB->getNoteIndices();

    beginInsertRows(QModelIndex(), noteIndexList.count(), noteIndexList.count() + (list.count() - 1));
    noteIndexList.append(list);
    endInsertRows();
}

int NoteListModel::rowCount(const QModelIndex &parent) const {
    return noteIndexList.count();
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid())
        return QVariant();

    if (index.row() >= noteIndexList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return noteIndexList.at(index.row()).title;
    }
    else if (role == Qt::UserRole) {

        // TODO: Optimize this
        int maxLength = 60;
        QString data = noteIndexList.at(index.row()).summary.trimmed();

        if (data.length() > maxLength) {
            int idx = data.indexOf("\n");
            if (idx < 0) idx = maxLength;
            data.truncate(qMin(idx, maxLength));
            data.append("...");
        }

        return data;
    }
    else return QVariant();
}


QVariant NoteListModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

void NoteListModel::appendNote(const Note &note) {
    if (!note.data.isEmpty())
    {
        // Return ID from database
        NoteIndex noteIndex = note.index;
        noteIndex.id = NoteDB->addNote(note);        // TODO: optimize by adding updateKey() function

        beginInsertRows(QModelIndex(), noteIndexList.count(), noteIndexList.count());
        noteIndexList.append(noteIndex);
        endInsertRows();
    }
}

void NoteListModel::appendNote(const NoteList &list) {

    if (list.count() > 0) {

        // Add notes to DB and return list of IDs
        QList<int> idList = NoteDB->addNotes(list);

        QList<NoteIndex> indexList;

        // Extract the index and assign IDs
        for(int i = -1; ++i < idList.count();) {
            NoteIndex index;
            index = list[i].index;
            index.id = idList[i];
            indexList.append(index);
        }

        // and insert them to the list model
        beginInsertRows(QModelIndex(), noteIndexList.count(), noteIndexList.count() + (indexList.count() - 1));
        noteIndexList.append(indexList);
        endInsertRows();

    }
}

void NoteListModel::insertNote(const Note &note, int row) {
    if (!note.data.isEmpty())
    {
        NoteIndex noteIndex = note.index;
        noteIndex.id = NoteDB->addNote(note);

        beginInsertRows(QModelIndex(), row, row);
        noteIndexList.insert(row, noteIndex);
        endInsertRows();
    }
}

void NoteListModel::editNote(const Note &note, int row) {
    if (!note.data.isEmpty())
    {
        NoteDB->updateNote(note);

        noteIndexList[row] = note.index;
        dataChanged(QModelIndex(), QModelIndex());
    }
}

void NoteListModel::removeNote(int row) {

    qDebug() << "Remove note DB: " << noteIndexList.at(row).id;

    NoteDB->removeNoteById(noteIndexList.at(row).id);

    beginRemoveRows(QModelIndex(), row, row);
    noteIndexList.removeAt(row);
    endRemoveRows();
}

Note NoteListModel::noteData(const QModelIndex &index) const {
    return NoteDB->getNoteById(noteIndexList.at(index.row()).id);
}

void NoteListModel::search(const QString &search) {

    beginRemoveRows(QModelIndex(), 0, noteIndexList.count() - 1);
    noteIndexList.clear();
    endRemoveRows();

    NoteIndexList list = NoteDB->getNoteIndices(search);

    beginInsertRows(QModelIndex(), noteIndexList.count(), noteIndexList.count() + (list.count() - 1));
    noteIndexList.append(list);
    endInsertRows();

}

void NoteListModel::refreshData() {

    beginRemoveRows(QModelIndex(), 0, noteIndexList.count() - 1);
    noteIndexList.clear();
    endRemoveRows();

    NoteIndexList list = NoteDB->getNoteIndices();

    if (!list.isEmpty()) {
        beginInsertRows(QModelIndex(), noteIndexList.count(), noteIndexList.count() + (list.count() - 1));
        noteIndexList.append(list);
        endInsertRows();
    }

}
