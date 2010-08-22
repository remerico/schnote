#include "api.h"

#include <iostream>
#include <string>

#include <QTextStream>
#include <QtDebug>
#include <QThread>

#include <QScriptValueIterator>
#include <QScriptEngine>

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
            _deleteNote(task.parameter.at(0).toString(), task.parameter.at(1).toBool());
            break;

        }

    }

    qDebug() << "API thread finished.";

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

void Api::deleteNote(const QString &key, bool permanent) {
    QList<QVariant> parameters;
    parameters.append(key);
    parameters.append(permanent);

    _enqueueTask(Task(Task::DELETE_NOTE, parameters));
}


//
//----------------------------------------------------------------


bool Api::_login() {

    qDebug() << "Logging in...";

    const char* url = "https://simple-note.appspot.com/api/login";

    CurlObject curl;

    curl.setProxy(_proxy);
    curl.setUrl(url);

    // Setup POST data
    curl.setEncodedPostData("email=" + _account.user + "&password=" + _account.password);
    curl.perform();

    if (curl.getResponseCode() == 200) {
        _account.token = curl.getResponseBody();
        emit acquiredToken(curl.getResponseBody());
        return true;
    }
    else {
        qDebug() << "Login ERROR! " << curl.getResponseCode();
        qDebug() << "Response: " << curl.getResponseBody();
        return false;
    }



}


NoteIndexHash Api::_getNoteIndices() {

    if (!_account.token.length()) _login();

    if (_account.token.length()) {

        QString url = "http://simple-note.appspot.com/api/index?auth=" + _account.token + "&email=" + _account.user;

        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toAscii().data());
        curl.perform();


        // Parse JSON data

        NoteIndexHash indexHash;


        QScriptEngine engine;
        QScriptValue scriptValue;

        //qDebug() << "Response: " << curl.getResponseBody();
        scriptValue = engine.evaluate(curl.getResponseBody());

        if (scriptValue.isArray()) {

            QScriptValueIterator itr(scriptValue);

            while (itr.hasNext()) {
                itr.next();

                NoteIndex noteIndex;

                noteIndex.key = itr.value().property("key").toString();
                noteIndex.modifyDate = fromApiDate(itr.value().property("modify").toString());
                noteIndex.deleted = itr.value().property("deleted").toBool();

                //qDebug() << "key:" << noteIndex.key << " modify:" << noteIndex.modifyDate << " deleted:" << noteIndex.deleted;

                indexHash.insert(noteIndex.key, noteIndex);
            }

        }

        qDebug() << "Note index hash count: " << indexHash.count();


        return indexHash;



        // TODO: Cleanup this code / separate JSON parsing to another class
        /*
        json_parser parser;
        JsonData<NoteIndexHash, NoteIndex> indexHash;

        if (json_parser_init(&parser, NULL, parseNoteIndex, &indexHash) == 0) {

            QString json_data = curl.getResponseBody();

            const char *c_data = json_data.toLatin1().data();
            int length = json_data.length();

            int ret = json_parser_string(&parser, c_data, length, NULL);
            if (ret) {
                std::cout << "JSON error: " << ret << "  data:  " << c_data << "\n";
            }

            json_parser_free(&parser);

        }

        if (curl.getResponseCode() == 200) {
            emit receivedNoteIndices(indexHash.data);
        }

        qDebug() << "API notes count: " << indexHash.data.count();

        return indexHash.data;
        */

    }


    return NoteIndexHash();

}


Note Api::_getNote(QString id) {

    Note note;

    if (!_account.token.length()) _login();

    if (_account.token.length()) {

        QString url = "http://simple-note.appspot.com/api/note?key=" + id + "&auth=" + _account.token + "&email=" + _account.user;

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


    if (!_account.token.length()) _login();

    if (_account.token.length()) {

        QString url = "http://simple-note.appspot.com/api/note?auth=" + _account.token +
                      "&email=" + _account.user +
                      (note.index.creationDate.isValid() ?
                            "&create=" + toApiDate(note.index.creationDate).replace(" ", "%20") : "") +
                      (note.index.modifyDate.isValid() ?
                            "&modify=" + toApiDate(note.index.modifyDate).replace(" ", "%20") : "");

        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toAscii().data());

        qDebug() << "Note data: " << note.data;


        // Setup POST data
        curl.setEncodedPostData(note.data);
        curl.perform();


        qDebug() << "Create response: " << curl.getResponseBody();

        if (curl.getResponseCode() == 200) {

            // Auto update DB record from here
            Note nt = note;
            nt.index.key = curl.getResponseBody();
            if (nt.index.id >= 0) NoteDB->updateNote(nt, false);

            emit createNoteFinished(curl.getResponseBody());
            return curl.getResponseBody();
        }
        else {
            qDebug() << "Create note ERROR! " << curl.getResponseCode();
        }

    }

    return QString();

}


bool Api::_updateNote(const Note &note) {

    qDebug() << "  Updating API note...";

    if (!_account.token.length()) _login();

    if (_account.token.length()) {

        QString url = "http://simple-note.appspot.com/api/note?key=" + note.index.key +
                      "&auth=" + _account.token +
                      "&email=" + _account.user +
                      (note.index.modifyDate.isValid() ? "&modify=" + toApiDate(note.index.modifyDate).replace(" ", "%20") : "" );

        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toAscii().data());

        qDebug() << "Note title: " << note.index.title;

        // Setup POST data
        qDebug() << "  Update data: " << note.data;

        curl.setEncodedPostData(note.data);
        curl.perform();

        qDebug() << "Update response: " << curl.getResponseBody();

        if (curl.getResponseCode() == 200) {
            return true;
        }
        else {
            qDebug() << "Update note ERROR! " << curl.getResponseCode();
        }

    }

    return false;

}


void Api::_syncNotes() {

    if (_account.token.isEmpty()) {
        qDebug() << "Account token is empty";
        _login();
    }

    if (_account.token.length()) {

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

                if (dbNote.modifyDate > apiNote.modifyDate) {

                    qDebug() << "\n\n" << dbNote.modifyDate << " > " << apiNote.modifyDate;
                    qDebug() << "DB note is newer than API";

                    if (dbNote.deleted && !apiNote.deleted) {
                        _deleteNote(dbNote.key);
                    }
                    else if (!dbNote.deleted && !apiNote.deleted) {
                        _updateNote(NoteDB->getNoteById(dbNote.id));
                    }

                }

                // Both client and server are marked as deleted
                // Delete permanently (unless iPhone compatibility
                // is enabled.)
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

                        // Delete API note permanently (unless iPhone compatibility is set)
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

                // Otherwise delete it permanently (unless iPhone compatibility is set)
                // EDIT: Doesn't work for now anyway. Stupid API >:P
                //else _deleteNote(apiNote.key, true);
            }

        }

        NoteDB->setAllSynchronized();

        emit syncNotesFinished();

    }

}


void Api::_deleteNote(const QString &key, bool permanent) {

    if (!_account.token.length()) _login();

    if (_account.token.length()) {

        QString url = "http://simple-note.appspot.com/api/delete?key=" + key +
                      "&auth=" + _account.token +
                      "&email=" + _account.user +
                      (permanent ? "&dead=1" : "");

        CurlObject curl;

        curl.setProxy(_proxy);
        curl.setUrl(url.toAscii().data());

        qDebug() << "Deleting API note: " << key << (permanent ? " permanently!" : "");

        curl.perform();

        qDebug() << "Delete response: " << curl.getResponseBody();

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

    QString dateStr(dateString);
    dateStr.truncate(19);    // force truncate milliseconds >:P

    QDateTime date = QDateTime::fromString(dateStr, "yyyy-MM-dd hh:mm:ss");
    date.setTimeSpec(Qt::UTC);

    //qDebug() << "from API date: " << dateStr << " ---> " << date;

    return date;
}

QString Api::toApiDate(const QDateTime &date) {

    QDateTime apiDate(date);
    apiDate.setTimeSpec(Qt::UTC);

    return apiDate.toString("yyyy-MM-dd hh:mm:ss");

}


//----------------------------------------------------------------
// JSON parse functions (clean this up!)
/*
int Api::parseNoteIndex(void *userdata, int type, const char *data, uint32_t length) {

    JsonData<NoteIndexHash, NoteIndex>* jsonData = (JsonData<NoteIndexHash, NoteIndex>*)userdata;

    switch (type) {
        case JSON_ARRAY_BEGIN:
        case JSON_ARRAY_END:
            break;

        case JSON_OBJECT_END:
            jsonData->data.insert(jsonData->tempData.key, jsonData->tempData);
            break;

        case JSON_OBJECT_BEGIN:
            jsonData->tempData = NoteIndex();
            break;

        if (!jsonData->data.isEmpty()) {

            case JSON_KEY:
                jsonData->lastKey = data;
                break;

            case JSON_STRING:
                if (jsonData->lastKey.compare("key") == 0) {
                    jsonData->tempData.key = data;
                }
                else if (jsonData->lastKey.compare("modify") == 0) {
                    jsonData->tempData.modifyDate = fromApiDate(data);
                }
                break;

            case JSON_TRUE:
            case JSON_FALSE:
                if (jsonData->lastKey.compare("deleted") == 0) {
                    jsonData->tempData.deleted = (type == JSON_TRUE);
                }
                break;

            case JSON_INT:
            case JSON_FLOAT:
            case JSON_NULL:
                break;

        } //end if

    }
    return 0;
}
*/
