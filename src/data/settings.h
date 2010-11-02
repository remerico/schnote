#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

#include "../lib/curlobject.h"
#include "../simplenote/api.h"

class SettingsImpl : public QObject
{
    Q_OBJECT
public:
    inline static SettingsImpl* getInstance() { return instance; }

    SimpleNote::Account getAccount();
    CurlProxy getProxy();
    QDateTime getLastSyncDate();

private:
    SettingsImpl(QObject *parent = 0);

    static SettingsImpl* instance;

signals:
    void accountChanged(const SimpleNote::Account &account);
    void proxyChanged(const CurlProxy &proxy);

public slots:
    void setAccount(const SimpleNote::Account &account);
    void setProxy(const CurlProxy &proxy);
    void setLastSyncDate(QDateTime dateTime);

};

#define Settings SettingsImpl::getInstance()

#endif // SETTINGS_H
