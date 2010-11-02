#if 0

#ifndef JSONOBJECT_H
#define JSONOBJECT_H

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <QMapIterator>
#include <QMetaType>

class JsonValue : public QObject
{
    Q_OBJECT
public:
    explicit JsonValue(QObject *parent = 0);
    JsonValue(const QString& value);

    void addProperty(const QString& key, const JsonValue& value);
    void addProperty(const QString &key, const QString& value);

    enum ObjectType {
        OBJECT,
        STRING,
        ARRAY
    };

    QString toJsonString();

private:
    ObjectType type;
    QMap<QString, QVariant> propertyList;
    QVariant value;

public slots:

};

Q_DECLARE_METATYPE( JsonValue )

#endif // JSONOBJECT_H

#endif
