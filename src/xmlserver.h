#ifndef XMLSERVER_H
#define XMLSERVER_H

#include <QTcpServer>

class QTcpServer;

class XMLServer : public QTcpServer
{
    Q_OBJECT

public:
    XMLServer(QObject *parent = 0);
    ~XMLServer();
    void start(int port);

signals:
    void connectionError(const QString &);
    void connected();
};

#endif
