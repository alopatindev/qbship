#include "mainwindow.h"
#include "createdialog.h"
#include "connectdialog.h"
#include "mapwidget.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QProcessEnvironment>

class MainWindow;
class CreateDialog;
class ConnectDialog;
class QMessageBox;
class QCloseEvent;
class QSettings;
class MapWidget;
class QProcessEnvironment;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tcpServerConnection(0), turn(0),
      ready(false), enemyReady(false)
{
    setupUi(this);
    // FIXME: magic numbers!
    setMinimumSize(MAP_WIDTH+340, MAP_WIDTH+100);
    Environ::instance()["user"] = \
        QProcessEnvironment::systemEnvironment().value("USER", "Unknown user");
    myUsername->setText(Environ::instance()["user"]);
    setMode(Disconnected);
    myTurn = false;
    connect(&server, SIGNAL(connectionError(const QString &)),
            this, SLOT(connectionError(const QString &)));
    connect(&server, SIGNAL(connected()), this, SLOT(connected()));
    connect(&server, SIGNAL(newConnection()), this, SLOT(processClient()));
    connect(&client, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(&client, SIGNAL(disconnected()), this, SLOT(disconnect()));
    connect(&client, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(myMap, SIGNAL(eventOccured(const QString &, const DataMap &)),
            this, SLOT(sendMessage(const QString &, const DataMap &)));
    connect(myMap, SIGNAL(ready(bool)),
            this, SLOT(setReady(bool)));

    connect(&client, SIGNAL(readyRead()),
            this, SLOT(processClientData2()));

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
    ) {
        setMode(Disconnected);
        updateStatus(tr("Disconnected"));
    }
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

void MainWindow::connectionEstablished()
{
    setMode(Client);
    myMap->setState(Edit); enemyMap->setState(View);
    updateStatus(tr("Connected to server. Let's play!"));
    myMap->setDisabled(false); enemyMap->setDisabled(false);
    sendMessage("who"); // FIXME: works unstable here
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
            turn = 0;
            break;
        case Client:
            actionDisconnect->setDisabled(false);
            server.close();
            myMap->clear();
            enemyMap->clear();
            enemyMap->setState(View);
            ready = false; enemyReady = false;
            turn = 0;
            break;
        case Server:
            actionDisconnect->setDisabled(false);
            if (client.state() == QAbstractSocket::ConnectedState)
                client.close();
            myMap->clear();
            enemyMap->clear();
            myMap->setDisabled(true); enemyMap->setDisabled(true);
            myMap->setState(Edit); enemyMap->setState(View);
            ready = false; enemyReady = false;
            turn = 0;
            break;
    }
}

void MainWindow::connected()
{
    updateStatus(tr("Connected"));
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

void MainWindow::processClient()
{
    updateStatus(tr("Client connected! Let's play!"));
    if (!server.hasPendingConnections())
        return;

    tcpServerConnection = server.nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(processClientData()));
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

void MainWindow::processClientData()
{
    getMessage(tcpServerConnection);
}

void MainWindow::processClientData2()
{
    getMessage(&client);
}

void MainWindow::getMessage(QTcpSocket *sock)
{
    int size = (int)(sock->bytesAvailable());
    QByteArray block = sock->readAll();
    qDebug("reading %d bytes", size);

    QDataStream in(&block, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_6);
    QString command; DataMap data;
    in >> command;
    in >> data;
    // TODO: process input data here
    qDebug("{{%s}}", command.toStdString().c_str());
    DataMap::const_iterator i;
    for (i = data.constBegin(); i != data.constEnd(); ++i)
        qDebug("[%s]: '%s'", i.key().toStdString().c_str(),
                             i.value().toStdString().c_str());

    DataMap outData;
    if (command == "who") {
        outData["user"] = Environ::instance()["user"];
        sendMessage("info", outData);
        return;
    }
    if (command == "info") {
        if (enemyUsername->text().isEmpty())
            enemyUsername->setText(data["user"]);
        return;
    }
    if (command == "ready") {
        enemyReady = true;
        checkBothReady();
        return;
    }
    if (command == "status") {
        if (enemyUsername->text().isEmpty())  // FIXME: ugly hack
            enemyUsername->setText(data["user"]);
        updateStatus(QString("%1: %2").arg(data["user"]).arg(data["text"]));
        return;
    }
    if (command == "turn") {
        if (running != Server)
            return;
        QString answer;
        switch (settings.firstPlayer) {
        case RandomPlayer: answer = qrand() % 2 ? "you" : "me"; break;
        case YouPlayer: answer = "me"; break;
        case OpponentPlayer: answer = "you"; break;
        }
        outData["turn"] = answer;
        sendMessage("wturn", outData);
        if (answer == "me") {
            if (turn == 0)
                myMap->setState(View);
            setMyTurn(true);
        }
        return;
    }
    if (command == "wturn") {
        if (running != Client)
            return;
        if (turn == 0)
            myMap->setState(View);
        if (data["turn"] == "you") {
            setMyTurn(true);
        } else if (data["turn"] == "me") {
            setMyTurn(false);
        }
        return;
    }
}

void MainWindow::sendMessage(const QString & command, const DataMap & data)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << command << data;

    if (running == Client)
        client.write(block);
    else if (running == Server && tcpServerConnection)
        tcpServerConnection->write(block);
}

void MainWindow::setMyTurn(bool my)
{
    updateStatus(tr(my ? "It's your turn!" : "It's enemy's turn."));
    enemyMap->setState(my ? Turn : View);
    myTurn = my;
    turn++;
}

void MainWindow::setReady(bool ready)
{
    this->ready = ready;
    sendMessage("ready");
    checkBothReady();
}

void MainWindow::checkBothReady()
{
    if (ready && enemyReady) {
        //if (running == Client)
            sendMessage("turn");
    }
}
