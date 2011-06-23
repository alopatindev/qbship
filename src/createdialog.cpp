#include "createdialog.h"

class CreateDialog;

CreateDialog::CreateDialog(GameSettings *settings, QWidget *parent)
    : QDialog(parent), settings(settings)
{
    setupUi(this);
    port->setValue(settings->serverPort);
    firstPlayer->setCurrentIndex((int)settings->firstPlayer);
}

CreateDialog::~CreateDialog()
{
}

void CreateDialog::accepted()
{
    settings->serverPort = port->value();
    settings->firstPlayer = (FirstPlayer)firstPlayer->currentIndex();
}
