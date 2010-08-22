#include "notedb.h"
#include <QtDebug>
#include <QDateTime>
#include <QVariant>

NoteDBImpl* NoteDBImpl::instance = new NoteDBImpl();

NoteDBImpl::NoteDBImpl(const QString &fileName, QObject *parent) : QObject(parent) {
    this->fileName = fileName;
    _createConnection();
}

int NoteDBImpl::addNote(Note note) {

    QDateTime currentDateTime = QDateTime::currentDateTime().toUTC();
    QSqlQuery query;

    if (!note.index.modifyDate.isValid()) note.index.modifyDate = currentDateTime;
    if (!note.index.creationDate.isValid()) note.index.creationDate = currentDateTime;

    query.prepare("insert into notes "
                  "(key, title, data, modifyDate, creationDate, deleted, changed) "
                  "values(:key, :title, :data, :modifyDate, :creationDate, :deleted, :changed)");

    query.bindValue(":key", note.index.key);
    query.bindValue(":title", note.index.title);
    query.bindValue(":data", note.data);
    query.bindValue(":modifyDate", note.index.modifyDate.toString(Qt::ISODate));
    query.bindValue(":creationDate", note.index.creationDate.toString(Qt::ISODate));
    query.bindValue(":deleted", (note.index.deleted ? "1" : "0"));
    query.bindValue(":changed", "1");

    if (query.exec()) {
        emit noteAdded(note);
        return query.lastInsertId().toInt();
    }

    return -1;

}

QList<int> NoteDBImpl::addNotes(NoteList noteList) {

    QList<int> idList;

    foreach(Note note, noteList) {
        idList.append(addNote(note));
    }

    return idList;
}

void NoteDBImpl::updateNote(Note note, bool updateTimeStamp) {
    if (note.index.id >= 0 || !note.index.key.isEmpty()) {
        QDateTime currentDateTime = QDateTime::currentDateTime().toUTC();
        QSqlQuery query;

        if (updateTimeStamp) note.index.modifyDate = currentDateTime;

        query.prepare("update notes set "
                      "key = :key, "
                      "title = :title, "
                      "data = :data, "
                      "modifyDate = :modifyDate, "
                      "creationDate = :creationDate, "
                      "deleted = :deleted, "
                      "changed = :changed "
                      "where id = :id "
                      "or key = :key2");

        query.bindValue(":key", note.index.key);
        query.bindValue(":title", note.index.title);
        query.bindValue(":data", note.data);
        query.bindValue(":modifyDate", note.index.modifyDate.toString(Qt::ISODate));
        query.bindValue(":creationDate", note.index.creationDate.toString(Qt::ISODate));
        query.bindValue(":deleted", (note.index.deleted ? "1" : "0"));
        query.bindValue(":changed", "1");
        query.bindValue(":id", note.index.id);
        query.bindValue(":key2", note.index.key);

        query.exec();

        emit noteUpdated(note);
        qDebug() << "DB note updated: " << note.index.title;
    }
}

void NoteDBImpl::removeNoteById(int id, bool permanent) {
    QSqlQuery query;

    QString noteKeyToBeDeleted = NoteDB->getNoteIndexById(id).key;

    if (permanent) {
        query.prepare("delete from notes where id = :id");
    }
    else {
        query.prepare("update notes "
                      "set deleted = '1', "
                      "modifyDate = :modifyDate, "
                      "changed = '1' "
                      "where id = :id");

        QDateTime currentDateTime = QDateTime::currentDateTime().toUTC();
        query.bindValue(":modifyDate", currentDateTime.toString(Qt::ISODate));
    }

    query.bindValue(":id", QString::number(id));
    query.exec();

    if (!noteKeyToBeDeleted.isEmpty()) emit noteKeyDeleted(noteKeyToBeDeleted);

}

NoteIndexList NoteDBImpl::_getIndicesFromQuery(const QString &queryString) {

    NoteIndexList noteIdxList;

    QSqlQuery query;
    query.exec(queryString);


    while (query.next()) {
        NoteIndex noteIdx;

        noteIdx.id           = query.value(0).toInt();
        noteIdx.key          = query.value(1).toString();
        noteIdx.setTitleSummary(query.value(2).toString());
        noteIdx.modifyDate   = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        noteIdx.creationDate = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
        noteIdx.deleted      = query.value(5).toBool();
        noteIdx.changed      = query.value(6).toBool();

        noteIdx.modifyDate.setTimeSpec(Qt::UTC);
        noteIdx.creationDate.setTimeSpec(Qt::UTC);

        noteIdxList.append(noteIdx);
    }

    qDebug() << "  Returned" << noteIdxList.count() << "indices from DB";

    return noteIdxList;

}


NoteIndexHash NoteDBImpl::getNoteIndicesWithKeys(bool changed) {
    QSqlQuery query;

    query.exec("select * from noteindices where key <> \"\"" + (changed ? QString(" and changed = '1'") : "")  );

    NoteIndexHash noteIdxHash;

    while (query.next()) {
        NoteIndex noteIdx;

        noteIdx.id           = query.value(0).toInt();
        noteIdx.key          = query.value(1).toString();
        noteIdx.setTitleSummary(query.value(2).toString());
        noteIdx.modifyDate   = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        noteIdx.creationDate = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
        noteIdx.deleted      = query.value(5).toBool();
        noteIdx.changed      = query.value(6).toBool();

        noteIdx.modifyDate.setTimeSpec(Qt::UTC);
        noteIdx.creationDate.setTimeSpec(Qt::UTC);

        noteIdxHash.insert(noteIdx.key, noteIdx);
    }

    qDebug() << "getNoteIndicesWithKeys count: " << noteIdxHash.count();

    return noteIdxHash;
}

NoteIndexList NoteDBImpl::getNoteIndices(const QString &search, bool includeDeleted) {

    // Sorry if this looks like a mess, cuz it is.
    return _getIndicesFromQuery("select id, key, substr(data, 1, 255), modifyDate, creationDate, deleted, changed from notes" +
                (!search.isEmpty() ? QString(" where data like '%%1%'").arg(search) : "") +
                (!includeDeleted ? QString((search.isEmpty() ? " where" : " and")) + " deleted = '0'" : "") +
                 " order by creationDate" );

}

NoteIndexList NoteDBImpl::getNoteIndices(bool includeDeleted) {
    return getNoteIndices("", includeDeleted);
}

NoteIndexList NoteDBImpl::getNoteIndicesWithoutKeys() {
    qDebug() << "getNoteIndicesWithoutKeys()";
    return _getIndicesFromQuery("select * from noteindices where key = \"\" or key is NULL");
}

NoteIndexList NoteDBImpl::getDeletedNoteIndices() {
    qDebug() << "getDeletedNoteIndices()";
    return _getIndicesFromQuery("select * from noteindices where deleted = '1'");
}

NoteIndex NoteDBImpl::getNoteIndexById(int id) {
    NoteIndexList indexList = _getIndicesFromQuery("select * from noteindices where id = '" + QString::number(id) + "'");
    if (!indexList.empty()) return indexList.at(0);
    else return NoteIndex();
}

Note NoteDBImpl::getNoteById(int id) {

    QSqlQuery query;

    query.exec(QString("select * from notes where id = '%1'").arg(QString::number(id)));

    Note note;

    if (query.next()) {

        note.index.id           = query.value(0).toInt();
        note.index.key          = query.value(1).toString();
        // value(2) == note title
        note.setNote(query.value(3).toString());
        note.index.modifyDate   = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
        note.index.creationDate = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
        note.index.deleted      = query.value(6).toBool();
        note.index.changed      = query.value(7).toBool();

        note.index.modifyDate.setTimeSpec(Qt::UTC);
        note.index.creationDate.setTimeSpec(Qt::UTC);

    }

    return note;

}

void NoteDBImpl::setAllSynchronized() {
    QSqlQuery query;
    query.exec("update notes set changed = '0'");
}

void NoteDBImpl::sendRefreshSignal() {
    emit dataRefreshed();
}

bool NoteDBImpl::_createConnection() {

    bool db_exists = QFile::exists(fileName);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);

    if (db.open()) {
        if (!db_exists) _createTables();
        return true;
    }
    else {
        return false;
    }
}

void NoteDBImpl::_createTables() {
    QSqlQuery query;

    query.exec("create table config(param varchar(200), value varchar(200))");

    query.exec("create table notes ("
                    "id integer primary key, "
                    "key int, "
                    "title text, "
                    "data text, "
                    "modifyDate text, "
                    "creationDate text, "
                    "deleted int, "
                    "changed int)");

    query.exec("create view noteindices as "
                    "select id, key, substr(data, 1, 255) as \"summary\", modifyDate, creationDate, deleted, changed from notes");
}
