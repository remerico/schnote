#include "notesync.h"

NoteSyncImpl* NoteSyncImpl::instance = new NoteSyncImpl();

NoteSyncImpl::NoteSyncImpl(QObject *parent) : QObject(parent) {


}

NoteList NoteSyncImpl::sync(NoteList local, NoteList remote) {


    foreach(Note note, local) {

        foreach(Note note, remote) {

        }

    }

    return NoteList();

}

Note NoteSyncImpl::compareNewer(const Note &note1, const Note &note2) {
    return (note1.index.modifyDate > note2.index.modifyDate) ? note1 : note2;
}
