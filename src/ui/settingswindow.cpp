#include "src/ui/settingswindow.h"
#include "ui_settingswindow.h"

#include "../data/settings.h"

settingswindow::settingswindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingswindow)
{
    ui->setupUi(this);


    SimpleNote::Account account = Settings->getAccount();
    CurlProxy proxy = Settings->getProxy();

    ui->email->setText(account.user);
    ui->password->setText(account.password);

    ui->proxySwitch->setChecked(!proxy.isEmpty());
    ui->proxyForm->setVisible(ui->proxySwitch->isChecked());
    ui->proxyUrl->setText(proxy.url);
    ui->proxyUser->setText(proxy.user);
    ui->proxyPassword->setText(proxy.password);

}

settingswindow::~settingswindow()
{
    delete ui;
}

void settingswindow::on_buttonBox_clicked(QAbstractButton* button)
{
    Settings->setAccount(SimpleNote::Account(ui->email->text(), ui->password->text()));
    Settings->setProxy(CurlProxy(ui->proxyUrl->text(), ui->proxyUser->text(), ui->proxyPassword->text()));

    close();
}

void settingswindow::on_proxySwitch_clicked(bool checked)
{
    ui->proxyForm->setVisible(checked);
}
