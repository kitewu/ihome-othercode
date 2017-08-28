#ifndef SOCKETCLASS_H
#define SOCKETCLASS_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>

class SocketClass :public QObject
{
    Q_OBJECT
public:
    explicit SocketClass(QObject *parent = 0);

    static SocketClass* GetSocket();

    void WriteToSocket(const QByteArray &byte);

    bool OpenSocket();

    bool CloseSocket();

    void ReleaseSocket();
    
signals:
    void SocketMsg(QByteArray byte, qint64 size);
    
public slots:
    void ReadFromSocket();

private:
    static SocketClass *socket_service_;
    QTcpSocket *my_socket_;
    enum{OPEN, CLOSE};
    int socket_state_;

private:

};

#endif // SOCKETCLASS_H
