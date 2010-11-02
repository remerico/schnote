#include "api.h"

#include <iostream>
#include <string>

#include <QTextStream>
#include <QtDebug>
#include <QThread>
#include <QUrl>

#include "../data/settings.h"

#define API2_BASE_URL(path) "https://simple-note.appspot.com/api2/" path

using namespace SimpleNote;


Api::Api() { }

Api::Api(const QString &username, const QString &password) {
    Account account(username, password);
    setAccount(account);
}


// API thread run function
void Api::run() {

    qDebug() << "API thread is running.";

    while(!_taskQueue.empty()) {

        Task task = _taskQueue.dequeue();

        switch(task.description) {

        case Task::LOGIN:
            _login();
            break;

        case Task::GET_NOTE_INDEX:
            _getNoteIndices();
            break;

        case Task::GET_NOTE:
            _getNote(task.parameter.at(0).toString());
            break;

        case Task::GET_NOTES:
            _getNotes();
            break;

        case Task::CREATE_NOTE:
            _createNote(qvariant_cast<Note>(task.parameter.at(0)));
            break;

        case Task::UPDATE_NOTE:
            _updateNote(qvariant_cast<Note>(task.parameter.at(0)));
            break;

        case Task::SYNC_NOTES:
            _syncNotes();
            break;

        case Task::DELETE_NOTE:
            _deleteNote(qvariant_cast<NoteIndex>(task.parameter.at(0)), task.parameter.at(1).toBool());
            break;

        }

    }

    qDebug() << "API thread finished.";

    // Clear token when all task are finished
    _token.clear();

}


//----------------------------------------------------------------
// All that these functions do is enqueue tasks to the API thread

void Api::login() {
    _enqueueTask(Task(Task::LOGIN));
}

void Api::getNoteIndex() {
    _enqueueTask(Task(Task::GET_NOTE_INDEX));
}

void Api::getNote(QString index) {
    _enqueueTask(Task(Task::GET_NOTE, index));
}

void Api::getNotes() {
    _enqueueTask(Task(Task::GET_NOTES));
}

void Api::createNote(const Note &note) {
    _enqueueTask(Task(Task::CREATE_NOTE, QVariant::fromValue(note)));
}

void Api::updateNote(const Note &note) {
    _enqueueTask(Task(Task::UPDATE_NOTE, QVariant::fromValue(note)));
}

void Api::syncNotes() {
    _enqueueTask(Task(Task::SYNC_NOTES));
}

void Api::deleteNote(const NoteIndex& noteIndex, bool permanent) {
    QList<QVariant> parameters;
    parameters.append(QVariant::fromValue(noteIndex));
    parameters.append(permanent);

    _enqueueTask(Task(Task::DELETE_NOTE, parameters));
}


//
//----------------------------------------------------------------


bool Api::_login() {

    if (!_token.isEmpty()) {
        qDebug() << "Using existing token...";
        return true;
    }

    if (_account.isEmpty()) {
        qDebug() << "Account is empty, cannot login.";
        return false;
    }


    qDebug() << "Logging in...";

    const char* url = "https://simple-note.appspot.com/api/login";

    CurlObject curl;

    curl.setProxy(_proxy);
    curl.setUrl(url);

    // Setup POST data
    curl.setEncodedPostData("email=" + _account.user + "&password=" + _account.password);
    curl.perform();

    if (curl.getResponseCode() == 200) {
        _token = curl.getResponseBody();
        qDebug() << "Token:" << _token;
        emit acquiredToken(curl.getResponseBody());
        return true;
    }
    else {
        qDebug() << "Login error: " << curl.getResponseCode();
        qDebug() << "Response: " << curl.getResponseBody();
        return false;
    }


}


NoteIndexHash Api::_getNoteIndices(QDateTime since) {

    NoteIndexHash indexHash;

    if (_login()) {

        int length = 20;        // 20 indices per request
        QString mark;


        // request until no more indices left
        do {

            QUrl url(API2_BASE_URL("index"));

            url.addQueryItem("auth", _token);
            url.addQueryItem("email", _account.user);
            url.addQueryItem("length", QString::number(length));
            if (!mark.isEmpty()) url.addQueryItem("mark", mark);
            if (!since.isNull()) url.addQueryItem("since", toApiDate(since));

            qDebug() << "Index URL:" << url.toString();

            CurlObject curl;

            curl.setProxy(_proxy);
            curl.setUrl(url.toString().toAscii().data());
            curl.perform();


            QScriptEngine engine;
            QScriptValue scriptValue;
            QScriptValue noteData;


            scriptValue = engine.evaluate("(" + curl.getResponseBody() + ")");

            //qDebug() << "count:" << scriptValue.property("count").toString();

            // if mark is not empty, there is another page left
            noteData = scriptValue.property("data");
            mark = scriptValue.property("mark").toString();


            if (noteData.isArray()) {

                QScriptValueIterator itr(noteData);

                while (itr.hasNext()) {
                    itr.next();

                    NoteIndex noteIndex = responseToNoteIndex(itr.value());

                    if (!noteIndex.key.isEmpty()) {
                        //qDebug() << "key:" << noteIndex.key << " modify:" << noteIndex.modifyDate << " deleted:" << noteIndex.deleted;
                        indexHash.insert(noteIndex.key, noteIndex);
                    }

                }

            }

            qDebug() << "Note index hash count: " << indexHash.count();


        } while (!mark.isEmpty());


    }


    return indexHash;

}


Note Api::_getNote(QString id) {

    Note note;

    if (_login()) {

        QString url = "http://simple-note.appspot.com/api/note?key=" + id + "&auth=" + _token + "&email=" + _account.user;

        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toAscii().data());
        curl.perform();

        note.index.key          = curl.getHeaderValue("note-key");
        note.index.creationDate = fromApiDate(curl.getHeaderValue("note-createdate"));
        note.index.modifyDate   = fromApiDate(curl.getHeaderValue("note-modifydate"));
        note.index.deleted      = (curl.getHeaderValue("note-deleted").compare("true", Qt::CaseInsensitive) == 0);

        note.setNote(curl.getResponseBody());

        if (curl.getResponseCode() == 200) {
            emit receivedNote(note);
        }

        qDebug() << "  Fetch note: " << note.index.title;

    }

    return note;

}

NoteList Api::_getNotes() {

    NoteList noteList;
    NoteIndexHash indexList = _getNoteIndices();

    foreach(NoteIndex index, indexList) {
        if (!index.deleted && !index.key.isEmpty()) {
            noteList.append(_getNote(index.key));
        }
    }

    if (!noteList.isEmpty()) {
        emit receivedNotes(noteList);
    }

    return noteList;

}

QString Api::_createNote(const Note &note) {

    if (_login()) {

        QUrl url(API2_BASE_URL("data"));
        url.addQueryItem("auth", _token);
        url.addQueryItem("email", _account.user);


        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toString().toAscii().data());

        qDebug() << "Note data: " << note.content;

        QString data = note.content;
        data.replace('"', "\"");

        curl.setPostData("{"
                        "\"content\" : \"" + data + "\"" +
                        (note.index.modifyDate.isNull() ? ", \"modifydate\" : \"" + toApiDate(note.index.modifyDate) + "\"" : "") +
                        (note.index.creationDate.isNull() ? ", \"createdate\" : \"" + toApiDate(note.index.creationDate) + "\"" : "") +
                        "}");

        curl.perform();


        qDebug() << "Create response: " << curl.getResponseBody();

        if (curl.getResponseCode() == 200) {

            QScriptEngine engine;
            QScriptValue value = engine.evaluate("(" + curl.getResponseBody() + ")");

            Note nt = note;
            nt.index.key = value.property("key").toString();
            if (nt.index.id >= 0) NoteDB->updateNote(nt, false);

            emit createNoteFinished(nt.index.key);
            return nt.index.key;
        }
        else {
            qDebug() << "Create note ERROR! " << curl.getResponseCode();
        }

    }

    return QString();

}


NoteIndex Api::_updateNote(const Note &note, bool &ok) {

    if (_login()) {

        qDebug() << "  Updating API note...";

        QUrl url(API2_BASE_URL("data/") + note.index.key);
        url.addQueryItem("auth", _token);
        url.addQueryItem("email", _account.user);


        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toString().toAscii().data());

        qDebug() << "Update Note: " << note.index.title;

        QString data = note.content;
        data.replace('"', "\"");

        curl.setPostData("{"
                        "\"content\" : \"" + data + "\"" +
                        (note.index.modifyDate.isNull() ? ", \"modifydate\" : \"" + toApiDate(note.index.modifyDate) + "\"" : "") +
                        (note.index.creationDate.isNull() ? ", \"createdate\" : \"" + toApiDate(note.index.creationDate) + "\"" : "") +
                        "\"deleted\" : \"" + (note.index.deleted ? "true" : "false") + "\"" +
                        "}");

        curl.perform();

        if (curl.getResponseCode() == 200) {
            qDebug() << "Note updated.";
            return responseToNoteIndex(curl.getResponseBody());
        }
        else {
            qDebug() << "Update note ERROR! " << curl.getResponseCode();            
        }

    }

    return NoteIndex();

}


void Api::_syncNotes() {

    if (_login()) {

        // Get new unsynchronized notes from DB
        NoteIndexList newNotes = NoteDB->getNoteIndicesWithoutKeys();

        // Get synchronized and updated notes from DB
        NoteIndexHash dbNotesWithKeys = NoteDB->getNoteIndicesWithKeys();

        // Get all notes from Simplenote API
        NoteIndexHash apiNotes = _getNoteIndices();


        //------------------------------------------------

        // Create new notes to server
        foreach(NoteIndex noteIndex, newNotes) {
            Note note = NoteDB->getNoteById(noteIndex.id);
            qDebug() << "Creating new note '" << noteIndex.title << "' to server...";

            note.index.key = _createNote(note);
            qDebug() << "  Received key: " << note.index.key;

            //NoteDB->updateNote(note);
        }


        // Send updated notes to server
        foreach(NoteIndex dbNote, dbNotesWithKeys) {

            if (apiNotes.contains(dbNote.key)) {
                NoteIndex apiNote = apiNotes.value(dbNote.key, dbNote);

                if (dbNote.modifyDate > apiNote.modifyDate && apiNote.deleted == false) {

                    qDebug() << "\n\n" << dbNote.modifyDate << " > " << apiNote.modifyDate;
                    qDebug() << "DB note is newer than API";


                    // DB note deleted
                    if (dbNote.deleted && !apiNote.deleted) {
                        _deleteNote(dbNote);
                    }

                    // Both existing
                    else if (!dbNote.deleted && !apiNote.deleted) {
                        _updateNote(NoteDB->getNoteById(dbNote.id));
                    }

                }

                // Both client and server are marked as deleted
                // Delete permanently
                else if (dbNote.deleted && apiNote.deleted) {                    
                    NoteDB->removeNoteById(dbNote.id, true);

                    // EDIT: Doesn't work for now anyway. Stupid API >:P
                    //_deleteNote(dbNote.key, true);
                }


            }

            // Note is no longer in the API and DB is marked as deleted
            else if (!apiNotes.contains(dbNote.key) && dbNote.deleted) {
                NoteDB->removeNoteById(dbNote.id, true);  // delete permanently
            }

        }


        // Retrieve updated notes from server
        foreach(NoteIndex apiNote, apiNotes) {

            if (dbNotesWithKeys.contains(apiNote.key)) {

                NoteIndex dbNote = dbNotesWithKeys.value(apiNote.key, apiNote);

                if (apiNote.modifyDate > dbNote.modifyDate) {

                    if (!apiNote.deleted) {
                        qDebug() << "\n\n" << apiNote.modifyDate << " > " << dbNote.modifyDate;
                        qDebug() << "API note [" << dbNote.key << "] is newer than DB, updating...";
                        Note note = _getNote(apiNote.key);

                        NoteDB->updateNote(note, false);
                    }

                    else if (apiNote.deleted) {
                        if (!dbNote.deleted) {
                            NoteDB->removeNoteById(dbNote.id, true);
                        }

                        // Delete API note permanently
                        // EDIT: Doesn't work for now anyway. Stupid API >:P
                        //_deleteNote(apiNote.key, true);
                    }
                }

            }

            // API note is not in the DB
            else if (!dbNotesWithKeys.contains(apiNote.key)) {

                // if API note is not marked as deleted, create it locally
                if (!apiNote.deleted) {
                    NoteDB->addNote(_getNote(apiNote.key));
                }

                // Otherwise delete it permanently
                // EDIT: Doesn't work for now anyway. Stupid API >:P
                //else _deleteNote(apiNote.key, true);
            }

        }

        NoteDB->setAllSynchronized();


        Settings->setLastSyncDate(QDateTime::currentDateTimeUtc());

        emit syncNotesFinished();

    }

}


void Api::_deleteNote(const NoteIndex& noteIndex, bool permanent) {

    if (_login()) {


        // Non-permanent delete
        if (!permanent || (!noteIndex.deleted && permanent)) {

            QUrl url(API2_BASE_URL("data/") + noteIndex.key);
            url.addQueryItem("auth", _token);
            url.addQueryItem("email", _account.user);

            CurlObject curl;

            curl.setProxy(_proxy);
            curl.setUrl(url.toString().toAscii().data());

            curl.setPostData("{\"deleted\" : 1}");

            qDebug() << "Deleting API note: " << noteIndex.key;

            curl.perform();

            qDebug() << "Delete response: " << curl.getResponseBody();

        }


        // permanent delete
        else if (permanent) {



            // WALA PA


        }


    }

}


//----------------------------------------------------------------
// Task queue functions

bool Api::_taskContains(Task::Description description) {

    for(int i = -1 ; ++i < _taskQueue.count();) {
        if (_taskQueue[i].description == description) return true;
    }

    return false;

}

void Api::_enqueueTask(const Task &task) {

    if (!_taskContains(task.description)) {
        _taskQueue.enqueue(task);
        start();
    }

}


//----------------------------------------------------------------
// Utility functions

QDateTime Api::fromApiDate(const QString& dateString) {


    QDateTime date = QDateTime::fromMSecsSinceEpoch( dateString.toFloat() * 1000 );
    date.setTimeSpec(Qt::UTC);

    //qDebug() << "from API date: " << dateString << " ---> " << date;

    return date;
}

QString Api::toApiDate(const QDateTime &date) {

    QDateTime apiDate(date);
    apiDate.setTimeSpec(Qt::UTC);

    return QString::number( ((float)date.toMSecsSinceEpoch() / 1000) );

}

NoteIndex Api::responseToNoteIndex(const QScriptValue& response) {

    NoteIndex noteIndex;

    noteIndex.key = response.property("key").toString();
    noteIndex.deleted = response.property("deleted").toBool();
    noteIndex.modifyDate = fromApiDate(response.property("modifydate").toString());
    noteIndex.creationDate = fromApiDate(response.property("createdate").toString());
    noteIndex.version = response.property("version").toInteger();
    noteIndex.minversion = response.property("minversion").toInteger();
    noteIndex.syncNum = response.property("syncnum").toInteger();

    // tags
    if (response.property("tags").isArray()) {
        QScriptValueIterator tagitr(response.property("tags"));
        while(tagitr.hasNext()) {
            tagitr.next();
            noteIndex.tags.append(tagitr.value().toString() + " ");
        }
    }

    // system tags
    if (response.property("systemtags").isArray()) {
        QScriptValueIterator tagitr(response.property("systemtags"));
        while(tagitr.hasNext()) {
            tagitr.next();
            noteIndex.systemTags.append(tagitr.value().toString() + " ");
        }
    }

    return noteIndex;

}

Note Api::responseToNote(const QScriptValue& response) {

    Note note;

    note.index = responseToNoteIndex(response);
    note.content = response.property("content");

    return note;

}

NoteIndex Api::responseToNoteIndex(const QString& response) {
    QScriptEngine engine;
    return responseToNoteIndex(engine.evaluate(response));
}

Note Api::responseToNote(const QString& response) {
    QScriptEngine engine;
    return responseToNote(engine.evaluate(response));
}
