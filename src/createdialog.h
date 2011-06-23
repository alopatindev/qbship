#ifndef CREATEDIALOG_H
#define CREATEDIALOG_H

#include "ui_createdialog.h"
#include "mainwindow.h"
#include <QDialog>

class Ui_CreateDialog;
struct GameSettings;

class CreateDialog : public QDialog, Ui_CreateDialog
{
    Q_OBJECT

    GameSettings *settings;

public:
    CreateDialog(GameSettings *settings, QWidget *parent = 0);
    ~CreateDialog();

public slots:
    void accepted();
};

#endif
