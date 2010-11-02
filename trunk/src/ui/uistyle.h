#ifndef UISTYLE_H
#define UISTYLE_H

#include <QStyle>
#include <QCommonStyle>

class UiStyle : public QStyle
{
    Q_OBJECT
public:
    explicit UiStyle(QObject *parent = 0);

signals:

public slots:

};

#endif // UISTYLE_H
