#ifndef NOTEDB_H
#define NOTEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>

#include "notedata.h"

static const QString DEFAULT_DB_FILENAME = "notes.db";

class NoteDBImpl : public QObject
{
    Q_OBJECT

public:
    inline static NoteDBImpl* getInstance() { return instance; }

    QList<int> addNotes(NoteList noteList);
    int addNote(Note note);
    void updateNote(Note note, bool updateTimeStamp = true);
    void removeNoteById(int id, bool permanent = false);
    NoteIndexHash getNoteIndicesWithKeys(bool changed = false);   // for (faster?) syncing purposes
    NoteIndexList getNoteIndicesWithoutKeys();                    // hehehe
    NoteIndexList getNoteIndices(bool includeDeleted = false);
    NoteIndexList getNoteIndices(const QString &search, bool includeDeleted = false);
    NoteIndexList getDeletedNoteIndices();
    Note getNoteById(int id);
    NoteIndex getNoteIndexById(int id);
    void setAllSynchronized();

    void sendRefreshSignal();


private:
    NoteDBImpl(const QString &fileName = DEFAULT_DB_FILENAME, QObject *parent = 0);

    static NoteDBImpl* instance;
    QString fileName;

    bool _createConnection();
    void _createTables();
    NoteIndexList _getIndicesFromQuery(const QString &queryString);

signals:
    void dataRefreshed();
    void noteAdded(const Note &note);
    void noteUpdated(const Note &note);
    void noteKeyDeleted(const QString &key);

public slots:

};

#define NoteDB NoteDBImpl::getInstance()

#endif // NOTEDB_H
