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

    void loadSettings();

    SimpleNote::Account getAccount();
    CurlProxy getProxy();

private:
    SettingsImpl(QObject *parent = 0);

    static SettingsImpl* instance;

    QSettings settings;

signals:
    void accountChanged(const SimpleNote::Account &account);
    void proxyChanged(const CurlProxy &proxy);

public slots:
    void setAccount(const SimpleNote::Account &account);
    void setProxy(const CurlProxy &proxy);

};

#define Settings SettingsImpl::getInstance()

#endif // SETTINGS_H
