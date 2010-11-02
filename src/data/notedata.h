#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <QString>
#include <QDateTime>
#include <QDataStream>
#include <QRegExp>
#include <QMetaType>
#include <QStringList>

struct NoteIndex {

    int id;                 // internal id used by Schnote database
    int version;            // Track note content changes, set by server
    int minversion;         // Minimum version available for note
    int syncNum;            // Track note changes, set by server

    bool deleted;           // Determines if note is marked as deleted
    QDateTime modifyDate;   // Note modifyDate
    QDateTime creationDate; // Note creation date

    QString key;            // Note key, set by server
    QString shareKey;       // Shared note identifier, set by server
    QString publishKey;     // Published note identifier, set by server

    QString tags;           // List of tags
    QString systemTags;     // List of system tags, some set by server

    QString title;          // Title of the note
    QString summary;        // Short version of note data (to conserve memory)

    bool changed;           // True if note has changed since last sync
    bool pinned;            // True if note is 'pinned'


    NoteIndex() {
        changed = false;
        deleted = false;
        id = -1;
    }

    void setTitleSummary(const QString &data) {

        QString gist = data.trimmed().left(255);

        int retPos = data.trimmed().indexOf(QRegExp("\r|\n"), 1);

        title   = (retPos > 0)  ? gist.left(retPos) : gist;
        summary = (retPos >= 0) ? gist.mid(retPos, 255).simplified() : "";

    }

    QStringList getTagList() {
        return tags.split(" ", QString::SkipEmptyParts);
    }

};

struct Note {

    // Note index for syncing with Simplenote
    NoteIndex index;

    // Note data
    QString content;


    Note() { }

    Note(const QString &text) {
        setNote(text);
    }

    void setNote(const QString &text) {
        content = text;
        index.setTitleSummary(text);
    }

};

Q_DECLARE_METATYPE ( Note )
Q_DECLARE_METATYPE ( NoteIndex )

typedef QList<Note> NoteList;
typedef QList<NoteIndex> NoteIndexList;
typedef QHash<QString, NoteIndex> NoteIndexHash;

//Q_DECLARE_METATYPE(NoteIndexHash)

#endif // NOTEDATA_H
