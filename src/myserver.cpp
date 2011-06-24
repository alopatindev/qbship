#include "myserver.h"
#include <QTcpSocket>

class MyServer;
class QTcpSocket;

MyServer::MyServer(QObject *parent)
    : QTcpServer(parent)
{
    setMaxPendingConnections(1);
}

MyServer::~MyServer()
{
}

void MyServer::start(int port)
{
    close();
    if (!listen(QHostAddress::Any, port))
        emit connectionError(tr("Unable to start the server: %1")
                                 .arg(errorString()));
    else
        emit connected();
}
