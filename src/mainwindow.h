#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include <QMap>
#include "ui_mainwindow.h"
#include "myserver.h"
#include "singleton.h"

class QMainWindow;
class Ui_MainWindow;
class QString;
class QStringList;
class QTcpSocket;
class MyServer;

//typedef QMap<QString, QString> DataMap;

enum FirstPlayer {RandomPlayer, YouPlayer, OpponentPlayer};

struct GameSettings
{
    int connectPort;
    int serverPort;
    QString connectHostname;
    FirstPlayer firstPlayer;
    QStringList hostnames;
};

class MainWindow : public QMainWindow, Ui_MainWindow
{
    Q_OBJECT

    GameSettings settings;
    MyServer server;
    QTcpSocket client, *tcpServerConnection;

    enum Running {Disconnected, Client, Server} running;
    bool myTurn;

    void setMode(Running mode);
    void loadSettings();
    void saveSettings() const;
    void getMessage(QTcpSocket *sock);

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_actionNew_triggered();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionAbout_triggered();
    void connectionError(const QString & text);
    void connectionError(QAbstractSocket::SocketError);
    void connectionEstablished();
    void connected();
    void disconnect();
    void updateStatus(const QString & text);
    void processClient();
    void processClientData();
    void processClientData2();
    void sendMessage(const DataMap & data);
    void clientDisconnected();

protected:
    void closeEvent(QCloseEvent *event);
};

#endif
