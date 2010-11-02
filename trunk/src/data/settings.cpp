#include "src/data/settings.h"

#ifdef Q_WS_WIN32
    #define SETTINGS QSettings settings("settings.ini", QSettings::IniFormat)
#else
    #define SETTINGS QSettings settings
#endif


SettingsImpl* SettingsImpl::instance = new SettingsImpl();

SettingsImpl::SettingsImpl(QObject *parent) :
    QObject(parent)
{
}

void SettingsImpl::setAccount(const SimpleNote::Account &account) {

    SETTINGS;

    settings.setValue("account/user", account.user);

    QByteArray pw;
    pw.append(account.password);
    settings.setValue("account/password", QString(pw.toBase64()));

    emit accountChanged(account);

}


void SettingsImpl::setProxy(const CurlProxy &proxy) {

    SETTINGS;

    settings.setValue("proxy/url", proxy.url);
    settings.setValue("proxy/user", proxy.user);
    QByteArray pw;
    pw.append(proxy.password);
    settings.setValue("proxy/password", QString(pw.toBase64()));

    emit proxyChanged(proxy);

}


void SettingsImpl::setLastSyncDate(QDateTime dateTime) {

    SETTINGS;

    settings.setValue("sync/lastsync", dateTime.toString());
}


SimpleNote::Account SettingsImpl::getAccount() {

    SETTINGS;

    SimpleNote::Account account;

    account.user = settings.value("account/user").toString();

    QByteArray pw = QByteArray::fromBase64(settings.value("account/password").toByteArray());
    account.password = QString(pw);

    return account;

}

CurlProxy SettingsImpl::getProxy() {

    SETTINGS;

    CurlProxy proxy;

    proxy.url = settings.value("proxy/url").toString();
    proxy.user = settings.value("proxy/user").toString();

    QByteArray pw = QByteArray::fromBase64(settings.value("proxy/password").toByteArray());
    proxy.password = QString(pw);

    return proxy;

}

QDateTime SettingsImpl::getLastSyncDate() {

    SETTINGS;

    return settings.value("sync/lastsync").toDateTime();

}
