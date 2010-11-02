/*
 *  curlobject.cpp
 *  Schnote
 *
 *  Created by Rem on 7/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "curlobject.h"

#include <QtDebug>

#if ENABLE_CURL

#define PROXY_PORT 3128L    //TODO: Hardcoded! :D

CurlObject::CurlObject() {

    _handle = curl_easy_init();

    curl_easy_setopt(_handle, CURLOPT_WRITEDATA, &_buffer);
    curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, writeCallback);

    curl_easy_setopt(_handle, CURLOPT_WRITEHEADER, &_headerList);
    curl_easy_setopt(_handle, CURLOPT_HEADERFUNCTION, headerCallback);

    // turn off SSL cert verification
    // doesn't work with Simplenote API server
    curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYPEER, 0);

    initialize();

}

CurlObject::~CurlObject() {
    curl_easy_cleanup(_handle);
    //qDebug() << "Curl object deleted!\n";
}

void CurlObject::setProxy(CurlProxy proxy) {
    if (!proxy.isEmpty()) {
        curl_easy_setopt(_handle, CURLOPT_PROXY, proxy.url.toUtf8().data());
        curl_easy_setopt(_handle, CURLOPT_PROXYPORT, PROXY_PORT);

        if (!proxy.user.isEmpty() || !proxy.password.isEmpty()) {
            QString proxyAuth(proxy.user + ":" + proxy.password);
            curl_easy_setopt(_handle, CURLOPT_PROXYUSERPWD, proxyAuth.toUtf8().data());
        }

        curl_easy_setopt(_handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
    }
}

void CurlObject::initialize() {
    _headerList.clear();
    _buffer.clear();
}

void CurlObject::setUrl(const char* url) {
    curl_easy_setopt(_handle, CURLOPT_URL, url);
    //qDebug() << "URL is " << url << "\n";
}

void CurlObject::setPostData(const QString& data) {
     curl_easy_setopt(_handle, CURLOPT_COPYPOSTFIELDS, data.constData());
     //qDebug() << "Post data is " << data << "\n";
}

void CurlObject::setEncodedPostData(const QString& data) {
    QByteArray base64(data.toUtf8().toBase64());
    curl_easy_setopt(_handle, CURLOPT_COPYPOSTFIELDS, base64.constData());
}

CURLcode CurlObject::perform() {
    _code = curl_easy_perform(_handle);

    if (_code) {
        qDebug() << "Error: CURL result is " << _code << "\n";
    }

    return _code;
}

size_t CurlObject::writeCallback(char *data, size_t size, size_t nmemb, QByteArray* buffer) {

    int result = 0;

    // Is there anything in the buffer?
    if (data && buffer) {
        result = size * nmemb;      // How much did we write?
        buffer->append(QString::fromUtf8(data, result).left(result));       // Append the data to the buffer
    }

    return result;
}

size_t CurlObject::headerCallback(char *data, size_t size, size_t nmemb, QList<QPair<QString, QString> >* headerList) {

    int result = 0;

    // Is there anything in the header?
    if (data && headerList) {

        QString headerStr(data);
        int sep = headerStr.indexOf(": ");

        if (sep > 0) {
            QString key = headerStr.left(sep);
            QString val = headerStr.mid(sep + 2).trimmed();
            headerList->append(QPair<QString, QString> (key, val));
        }

        result = size * nmemb;      // How much did we write?
    }

    return result;
}

QString CurlObject::getHeaderValue(const QString& key) {

    for(int i = -1; ++i < _headerList.count();) {
        if (_headerList[i].first.compare(key) == 0) {
            return _headerList[i].second;
        }
    }

    return "";
}

long CurlObject::getResponseCode() {
    long code;
    curl_easy_getinfo(_handle, CURLINFO_RESPONSE_CODE, &code);
    return code;
}

#else

CurlObject::CurlObject() { }
CurlObject::~CurlObject() { }
void CurlObject::initialize() { }
void CurlObject::setUrl(const char* url) { }
void CurlObject::setPostData(const char* data) { }

CURLcode CurlObject::perform() {
    return CURLcode();
}

#endif //ENABLE_CURL


