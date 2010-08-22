#ifndef NOTESYNC_H
#define NOTESYNC_H

#include <QObject>

#include "notedata.h"
#include "notedb.h"

struct NoteSyncItem {

    enum ChangeType {
        ADDED,
        UPDATED,
        DELETED
    };

    ChangeType changeType;
    Note note;

};

typedef QList<NoteSyncItem> NoteSyncList;


class NoteSyncImpl : public QObject
{
    Q_OBJECT

public:
    inline static NoteSyncImpl* getInstance() { return instance; }

    NoteList sync(NoteList local, NoteList remote);
    inline Note compareNewer(const Note& note1, const Note& note2);


private:
    NoteSyncImpl(QObject *parent = 0);

    static NoteSyncImpl* instance;

signals:

public slots:

};

#define NoteSync NoteSyncImpl::getInstance()

#endif // NOTESYNC_H
