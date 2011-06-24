#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class QTcpServer;

class MyServer : public QTcpServer
{
    Q_OBJECT

public:
    MyServer(QObject *parent = 0);
    ~MyServer();
    void start(int port);

signals:
    void connectionError(const QString &);
    void connected();
};

#endif
