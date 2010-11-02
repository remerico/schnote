#ifndef SIMPLENOTEAPI_H
#define SIMPLENOTEAPI_H

#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QQueue>
#include <QVariant>
#include <QScriptValueIterator>
#include <QScriptEngine>

#include <vector>
#include <string>

#include "../lib/curlobject.h"
#include "../data/notedata.h"
#include "../data/notedb.h"


namespace SimpleNote {

    struct Account {

        Account() { }
        Account(const QString &user, const QString &password) {
            this->user = user;
            this->password = password;
        }

        QString user;
        QString password;

        bool isEmpty() { return user.isEmpty() || password.isEmpty(); }
    };

    class Api : public QThread
    {
        Q_OBJECT


        // Contains an API task and an optional parameter
        struct Task {

            enum Description {
                LOGIN,
                GET_NOTE_INDEX,
                GET_NOTE,
                GET_NOTES,
                CREATE_NOTE,
                UPDATE_NOTE,
                SYNC_NOTES,
                DELETE_NOTE
            };

            Task(Description description) { this->description = description; }
            Task(Description description, const QVariant &parameter)  {
                this->description = description;

                QList<QVariant> param;
                param.append(parameter);

                this->parameter = param;
            }
            Task(Description description, const QList<QVariant> &parameter)  {
                this->description = description;
                this->parameter = parameter;
            }

            Description description;
            QList<QVariant> parameter;

        };


        enum ErrorCode {
            INVALID_ACCOUNT,
            INVALID_NOTE,
            SEREVER_ERROR,
            CONNECTION_ERROR
        };

    private:

        Account _account;
        QString _token;
        QQueue<Task> _taskQueue;

        CurlProxy _proxy;

        bool _login();
        NoteIndexHash _getNoteIndices(QDateTime since = QDateTime());
        Note _getNote(QString index);
        NoteList _getNotes();
        QString _createNote(const Note& note);
        NoteIndex _updateNote(const Note& note, bool &ok = true);
        void _syncNotes();
        void _deleteNote(const NoteIndex& noteIndex, bool permanent = false);

        // Thread task functions
        bool _taskContains(Task::Description description);
        void _enqueueTask(const Task &task);

        // Utility functions
        static QDateTime fromApiDate(const QString& dateString);
        static QString toApiDate(const QDateTime& date);
        static NoteIndex responseToNoteIndex(const QScriptValue& response);
        static Note responseToNote(const QScriptValue& response);
        static NoteIndex responseToNoteIndex(const QString& response);
        static Note responseToNote(const QString& response);


    public:
        Api();
        Api(const QString &username, const QString &password);

        void login();
        void getNoteIndex();
        void getNote(QString id);
        void getNotes();        
        void syncNotes();

        inline Account& getAccount() { return _account; }

    public slots:
        void createNote(const Note& note);
        void updateNote(const Note& note);
        void deleteNote(const NoteIndex& noteIndex, bool permanent = false);

        void setAccount(const SimpleNote::Account &account) { _account = account; }
        void setAccount(const QString &username, const QString &password) { _account = Account(username, password); }
        void setProxy(const CurlProxy &proxy) { _proxy = proxy; }

    protected:
        virtual void run();

    signals:
        void acquiredToken(const QString &token);
        void receivedNoteIndices(const NoteIndexHash &indexList);
        void receivedNote(const Note &note);
        void receivedNotes(const NoteList &noteList);
        void createNoteFinished(const QString& key);
        void syncNotesFinished();

    };
		
}


#endif // SIMPLENOTEAPI_H
