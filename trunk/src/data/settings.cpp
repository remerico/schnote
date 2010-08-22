#include "src/data/settings.h"

SettingsImpl* SettingsImpl::instance = new SettingsImpl();

SettingsImpl::SettingsImpl(QObject *parent) :
    QObject(parent)
{
    settings.setDefaultFormat(QSettings::IniFormat);
}

void SettingsImpl::loadSettings() {


}

void SettingsImpl::setAccount(const SimpleNote::Account &account) {

    settings.setValue("account/user", account.user);
    settings.setValue("account/password", account.password);

    emit accountChanged(account);

}


void SettingsImpl::setProxy(const CurlProxy &proxy) {

    settings.setValue("proxy/url", proxy.url);
    settings.setValue("proxy/user", proxy.user);
    settings.setValue("proxy/password", proxy.password);

    emit proxyChanged(proxy);

}

SimpleNote::Account SettingsImpl::getAccount() {

    SimpleNote::Account account;

    account.user = settings.value("account/user").toString();
    account.password = settings.value("account/password").toString();

    return account;

}

CurlProxy SettingsImpl::getProxy() {

    CurlProxy proxy;

    proxy.url = settings.value("proxy/url").toString();
    proxy.user = settings.value("proxy/user").toString();
    proxy.password = settings.value("proxy/password").toString();

    return proxy;

}
