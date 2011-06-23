#include "xmlserver.h"
#include <QTcpSocket>

class XMLServer;
class QTcpSocket;

XMLServer::XMLServer(QObject *parent)
    : QTcpServer(parent)
{
    setMaxPendingConnections(1);
}

XMLServer::~XMLServer()
{
}

void XMLServer::start(int port)
{
    close();
    if (!listen(QHostAddress::Any, port))
        emit connectionError(tr("Unable to start the server: %1")
                                 .arg(errorString()));
    else
        emit connected();
}
