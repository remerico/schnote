#ifndef CURLOBJECT_H
#define CURLOBJECT_H

#define ENABLE_CURL 1

#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include <QStringList>

#include <string>
#include <iostream>


#if ENABLE_CURL
    #include <curl/curl.h>
#else
    typedef int CURLcode;
    typedef int CURL;
#endif

struct CurlProxy {
    CurlProxy() { }
    CurlProxy(const QString &url, const QString &user, const QString &password) {
        this->url = url;
        this->user = user;
        this->password = password;
    }

    QString url;
    QString user;
    QString password;
    bool isEmpty() { return url.isEmpty(); }
};

class CurlObject {
	
public:
    CurlObject();
    ~CurlObject();

    void initialize();

    void setUrl(const char* url);
    void setPostData(const char* data);
    void setEncodedPostData(const QString& data);

    inline QString getResponseBody() { return QString(_buffer); };
    QString getHeaderValue(const QString& key);
    CURLcode perform();
    long getResponseCode();

public slots:
    void setProxy(CurlProxy proxy);

private:

    QByteArray _buffer;
    QList<QPair<QString, QString> > _headerList;

    #if ENABLE_CURL

        CURL* _handle;
        CURLcode _code;

        static char* proxyUrl;
        static char* proxyUserPwd;

        static size_t writeCallback(char *data, size_t size, size_t nmemb, QByteArray* buffer);
        static size_t headerCallback(char *data, size_t size, size_t nmemb, QList<QPair<QString, QString> >* _headerList);

    #endif //ENABLE_CURL

};

#endif //CURLOBJECT_H
