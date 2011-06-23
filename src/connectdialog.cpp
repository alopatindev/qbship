#include "connectdialog.h"

class ConnectDialog;

ConnectDialog::ConnectDialog(GameSettings *settings, QWidget *parent)
    : QDialog(parent), settings(settings)
{
    setupUi(this);
    hostname->setText(settings->connectHostname);
    completer = new QCompleter(settings->hostnames, this);
    hostname->setCompleter(completer);
    port->setValue(settings->connectPort);
}

ConnectDialog::~ConnectDialog()
{
}

void ConnectDialog::accepted()
{
    settings->connectHostname = hostname->text();
    settings->connectPort = port->value();
    if (settings->hostnames.indexOf(hostname->text()) == -1)
        settings->hostnames.append(hostname->text());
}

void ConnectDialog::hostnameEdited()
{
    // FIXME: it'd be better to disable only "ok" button
    buttonBox->setDisabled(hostname->text().isEmpty());
}
