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
    bool ready, enemyReady, myTurn;
    int turn;
    QString command; DataMap data;
    FirstPlayer firstPlayed;

    enum Running {Disconnected, Client, Server} running;

    void setMode(Running mode);
    void loadSettings();
    void saveSettings() const;
    void processCommand(const QString & command,
                        const DataMap & data = DataMap());
    void setMyTurn(bool my);
    bool serversFirstTurn();

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
    void clientConnected();
    void clientDisconnected();
    void connectedToServer();
    void disconnect();
    inline void updateStatus(const QString & text);
    void getMessage(QTcpSocket *sock);
    void getClientMessage();
    void getServerMessage();
    void sendMessage(const QString & command = "",
                     const DataMap & data = DataMap());
    void sendMessageLater(const QString & command, const DataMap & data,
                          int timeOut);
    void attackEnemy(int i, int j);
    void setReady(bool ready);
    void checkBothReady();

protected:
    void closeEvent(QCloseEvent *event);
};

#endif
