#include <QtGui/QApplication>
#include "ui/mainwindow.h"
#include "data/settings.h"
#include <QDateTime>

#include <QtDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationDomain("schnote.org");
    a.setApplicationName("Schnote");


    MainWindow w;

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
