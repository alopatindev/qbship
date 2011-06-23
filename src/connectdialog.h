#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include "ui_connectdialog.h"
#include "mainwindow.h"
#include <QDialog>
#include <QCompleter>

class Ui_ConnectDialog;
class MainWindow;
class GameSettings;
class QDialog;
class QCompleter;

class ConnectDialog : public QDialog, Ui_ConnectDialog
{
    Q_OBJECT

    GameSettings *settings;
    QCompleter *completer;

public:
    ConnectDialog(GameSettings *settings, QWidget *parent = 0);
    ~ConnectDialog();

public slots:
    void accepted();
    void hostnameEdited();
};

#endif
