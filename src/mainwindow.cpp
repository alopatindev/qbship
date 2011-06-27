#include "mainwindow.h"
#include "createdialog.h"
#include "connectdialog.h"
#include "mapwidget.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QProcessEnvironment>
#include <QTimer>

class MainWindow;
class CreateDialog;
class ConnectDialog;
class QMessageBox;
class QCloseEvent;
class QSettings;
class MapWidget;
class QProcessEnvironment;
class QTimer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tcpServerConnection(0), /*turn(0), myTurn(false),*/
      ready(false), enemyReady(false)
{
    setupUi(this);
    // FIXME: magic numbers!
    setMinimumSize(MAP_WIDTH+340, MAP_WIDTH+100);
    Environ::instance()["user"] = \
        QProcessEnvironment::systemEnvironment().value("USER", "Unknown user");
    myUsername->setText(Environ::instance()["user"]);
    setMode(Disconnected);
    connect(&server, SIGNAL(connectionError(const QString &)),
            this, SLOT(connectionError(const QString &)));
    //connect(&server, SIGNAL(connected()), this, SLOT(serverConnected()));
    connect(&server, SIGNAL(newConnection()), this, SLOT(clientConnected()));
    connect(&client, SIGNAL(connected()), this, SLOT(connectedToServer()));
    connect(&client, SIGNAL(disconnected()), this, SLOT(disconnect()));
    connect(&client, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(myMap, SIGNAL(eventOccured(const QString &, const DataMap &)),
            this, SLOT(sendMessage(const QString &, const DataMap &)));
    connect(myMap, SIGNAL(ready(bool)),
            this, SLOT(setReady(bool)));
    connect(enemyMap, SIGNAL(attack(int, int)),
            this, SLOT(attackEnemy(int, int)));

    connect(&client, SIGNAL(readyRead()), this, SLOT(getClientMessage()));

    loadSettings();
    myMap->setState(Edit);
    enemyMap->setState(View);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (running && QMessageBox::question(
            this,
            tr("Exit the game"),
            tr("The game is running. Are you sure you wanna exit?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        ) != QMessageBox::Yes
    )
        event->ignore();
    else
        saveSettings();
}

void MainWindow::loadSettings()
{
    QSettings s(qApp->applicationName());
    QVariant data;
    
    data = s.value("connectPort");
    settings.connectPort = data.isValid() ? data.toInt() : 4455;

    data = s.value("serverPort");
    settings.serverPort = data.isValid() ? data.toInt() : 4455;

    data = s.value("connectHostname");
    settings.connectHostname = data.isValid() ? data.toString() : "";

    data = s.value("firstPlayer");
    settings.firstPlayer = data.isValid() ? (FirstPlayer)data.toInt()
                                          : RandomPlayer;

    data = s.value("hostnames");
    settings.hostnames = data.isValid() ? data.toStringList() : QStringList();
}

void MainWindow::saveSettings() const
{
    QSettings s(qApp->applicationName());
    s.setValue("connectPort", settings.connectPort);
    s.setValue("serverPort", settings.serverPort);
    s.setValue("firstPlayer", settings.firstPlayer);
    s.setValue("connectHostname", settings.connectHostname);
    s.setValue("hostnames", settings.hostnames);
    s.sync();
}

void MainWindow::on_actionNew_triggered()
{
    if (running && QMessageBox::question(
            this,
            tr("Create a new game"),
            tr("The game is running. Are you sure you want to stop it?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        ) != QMessageBox::Yes
    )
        return;

    CreateDialog dialog(&settings, this);
    if (dialog.exec() == QDialog::Accepted) {
        dialog.accepted();  // FIXME: why should I do that manually?
        setMode(Server);
        server.start(settings.serverPort);
        updateStatus(tr("Server started. Wait for client, please."));
    }
}

void MainWindow::on_actionConnect_triggered()
{
    if (running && QMessageBox::question(
            this,
            tr("Connect the game"),
            tr("The game is running. Are you sure you want to stop it?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        ) != QMessageBox::Yes
    )
        return;

    ConnectDialog dialog(&settings, this);
    if (dialog.exec() == QDialog::Accepted) {
        dialog.accepted();  // FIXME: why should I do that manually?
        setMode(Client);
        client.connectToHost(settings.connectHostname, settings.connectPort);
        updateStatus(tr("Establishing connection..."));
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    if (QMessageBox::question(
            this,
            tr("Disconnect"),
            tr("Are you sure you wanna disconnect the game?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        ) == QMessageBox::Yes
    )
        disconnect();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       tr("About QBShip"),
                       tr("This program is free software and GPLv3 licensed.\n\
Coded by Lopatin Alexander."));
}

void MainWindow::connectionError(const QString & text)
{
    setMode(Disconnected);
    QMessageBox::critical(this, tr("Connection error"), text);
}

void MainWindow::connectionError(QAbstractSocket::SocketError)
{
    connectionError(client.errorString());
}

void MainWindow::connectedToServer()
{
    setMode(Client);
    myMap->setState(Edit); enemyMap->setState(View);
    updateStatus(tr("Connected to server. Let's play!"));
    myMap->setDisabled(false); enemyMap->setDisabled(false);
    sendMessageLater("who", DataMap(), 1500);  // FIXME: works unstable here
}

void MainWindow::setMode(Running mode)
{
    running = mode;
    switch (mode) {
        case Disconnected:
            actionDisconnect->setDisabled(true);
            if (client.state() == QAbstractSocket::ConnectedState)
                client.close();
            server.close();
            myMap->setDisabled(true); enemyMap->setDisabled(true);
            myMap->clear(); enemyMap->clear();
            myMap->setState(View); enemyMap->setState(View);
            enemyUsername->setText("");
            myScore->setText("0");
            enemyScore->setText("0");
            ready = false; enemyReady = false;
            myTurn = false;
            turn = 0;
            break;
        case Client:
            actionDisconnect->setDisabled(false);
            server.close();
            myMap->clear();
            enemyMap->clear();
            enemyMap->setState(View);
            ready = false; enemyReady = false;
            myTurn = false;
            turn = 0;
            break;
        case Server:
            actionDisconnect->setDisabled(false);
            //if (client.state() == QAbstractSocket::ConnectedState)
            //    client.close();
            myMap->clear();
            enemyMap->clear();
            myMap->setDisabled(true); enemyMap->setDisabled(true);
            myMap->setState(Edit); enemyMap->setState(View);
            ready = false; enemyReady = false;
            myTurn = false;
            turn = 0;
            break;
    }
}

void MainWindow::disconnect()
{
    setMode(Disconnected);
    updateStatus(tr("Disconnected"));
}

void MainWindow::updateStatus(const QString & text)
{
    statusBar()->showMessage(text);
}

void MainWindow::clientConnected()
{
    updateStatus(tr("Client connected! Let's play!"));
    if (!server.hasPendingConnections())
        return;

    tcpServerConnection = server.nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(getServerMessage()));
    connect(tcpServerConnection, SIGNAL(disconnected()),
            this, SLOT(clientDisconnected()));
    
    myMap->setDisabled(false); enemyMap->setDisabled(false);
    sendMessage("who");
}

void MainWindow::clientDisconnected()
{
    tcpServerConnection->deleteLater();
    tcpServerConnection = 0;
    updateStatus(tr("Client disconnected!"));
    setMode(Server);
}

void MainWindow::getServerMessage()
{
    getMessage(tcpServerConnection);
}

void MainWindow::getClientMessage()
{
    getMessage(&client);
}

void MainWindow::getMessage(QTcpSocket *sock)
{
    //int size = (int)(sock->bytesAvailable());
    //qDebug("reading %d bytes", size);
    QByteArray block = sock->readAll();

    QDataStream in(&block, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_6);
    QString command; DataMap data;
    in >> command;
    in >> data;
    qDebug("{{%s}}", command.toStdString().c_str());
    DataMap::const_iterator i;
    for (i = data.constBegin(); i != data.constEnd(); ++i)
        qDebug("[%s]: '%s'", i.key().toStdString().c_str(),
                             i.value().toString().toStdString().c_str());
    processCommand(command, data);
}

void MainWindow::processCommand(const QString & command, const DataMap & data)
{
    qDebug("processing command '%s'", command.toStdString().c_str());
    DataMap d;

    //FIXME
    if (running == Client) {
        if (enemyUsername->text().isEmpty())
            sendMessage("who");
    }

    if (command == "turn") {
        if (running == Client) {
            if (data["turn"] == "client") {
                setMyTurn(true);
            } else {
                setMyTurn(false);
                updateStatus("Opponent is thinking now");
            }
        }
        return;
    }
    if (command == "attack") {
        qDebug("input attack");
        if (myTurn) {  // cheating?
            qDebug("CHEATING??");
            return;
        }
        int i = data["i"].toInt();
        int j = data["j"].toInt();
        qDebug("input attack coords (%d,%d)", i, j);
        if (myMap->isStruck(i, j)) {
            d["i"] = i; d["j"] = j;
            myMap->strike(i, j, true);
            if (myMap->isKilled(i, j, d)) {
                sendMessage("killed", d);
            } else {
                sendMessage("hurt", d);
            }
        } else {
            qDebug("WATER!");
            d["i"] = i; d["j"] = j;
            myMap->strike(i, j, false);
            sendMessage("miss", d);
            setMyTurn(true);
        }
        return;
    }
    if (command == "miss") {
        enemyMap->strike(data["i"].toInt(), data["j"].toInt(), false);
        setMyTurn(false);
        return;
    }
    if (command == "killed") {
        //TODO
        qDebug("KILLED!");
        enemyMap->strike(data["i"].toInt(), data["j"].toInt(), true);
        setMyTurn(true);
        return;
    }
    if (command == "hurt") {
        //TODO
        enemyMap->strike(data["i"].toInt(), data["j"].toInt(), true);
        setMyTurn(true);
        return;
    }
    if (command == "who") {
        d["user"] = Environ::instance()["user"];
        sendMessage("user_info", d);
        return;
    }
    if (command == "user_info") {
        qDebug("PROCeSSING user_info: %s", data["user"].toString().toStdString().c_str());
        enemyUsername->setText(data["user"].toString());
        return;
    }
    if (command == "status") {
        if (data["ready"] == true) {
            enemyReady = true;
            checkBothReady();
        }
        updateStatus(QString("%1: %2").arg(data["user"].toString())
                                      .arg(data["text"].toString()));
        return;
    }
}

void MainWindow::sendMessage(const QString & command, const DataMap & data)
{
    if (command.isEmpty()) {
        sendMessage(this->command, this->data);
        return;
    }

    qDebug(">>> sending message '%s'", command.toStdString().c_str());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << command << data;

    if (running == Client)
        client.write(block);
    else if (running == Server && tcpServerConnection)
        tcpServerConnection->write(block);
}

void MainWindow::sendMessageLater(const QString & command,
                                  const DataMap & data,
                                  int timeOut)
{
    this->command = command;
    this->data = data;
    QTimer::singleShot(timeOut, this, SLOT(sendMessage()));
}

void MainWindow::setMyTurn(bool my)
{
    updateStatus(tr(my ? "It's your turn!" : "It's enemy's turn."));
    enemyMap->setState(my ? Turn : View);
    myTurn = my;
    turn++;

    /*DataMap d;
    d["turn"] = running == Server ? "client" : "server";
    sendMessage("turn", d);*/
}

void MainWindow::setReady(bool ready)
{
    this->ready = ready;
    checkBothReady();
}

bool MainWindow::serversFirstTurn()
{
    qDebug("serversFirstTurn() turn=%d", turn);
    if (turn != 0)
        return false;

    if (settings.firstPlayer == RandomPlayer)
        return qrand() % 2;
    else
        return settings.firstPlayer == YouPlayer;
}

void MainWindow::checkBothReady()
{
    if (ready && enemyReady && running == Server) {
        bool s = serversFirstTurn();
        if (!s) {
            DataMap d; d["turn"] = "client";
            sendMessage("turn", d);
        }
        setMyTurn(s);
    }
}

void MainWindow::attackEnemy(int i, int j)
{
    qDebug("ATTACKING enemy i=%d j=%d", i, j);
    DataMap d; d["i"] = i; d["j"] = j;
    sendMessage("attack", d);
    enemyMap->setState(View);
    //setMyTurn(false);
}
