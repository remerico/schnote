#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QAbstractButton>

#include "../simplenote/api.h"
#include "../lib/curlobject.h"

namespace Ui {
    class settingswindow;
}

class settingswindow : public QDialog
{
    Q_OBJECT

public:
    explicit settingswindow(QWidget *parent = 0);
    ~settingswindow();

private:
    Ui::settingswindow *ui;

private slots:
    void on_proxySwitch_clicked(bool checked);
    void on_buttonBox_clicked(QAbstractButton* button);
};

#endif // SETTINGSWINDOW_H
