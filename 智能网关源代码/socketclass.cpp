#include "socketclass.h"
#include "mainwindow.h"

SocketClass::SocketClass(QObject *parent):
    QObject(parent),
    my_socket_(0),
    socket_state_(CLOSE)
{
}

SocketClass* SocketClass::socket_service_(0);


SocketClass *SocketClass::GetSocket()
{
    if(0 == socket_service_)
    {
        socket_service_ = new SocketClass();
    }
    return socket_service_;
}

void SocketClass::ReadFromSocket()
{
    if(socket_state_ == OPEN&&my_socket_)
    {
        QByteArray byte;
        byte = my_socket_->readAll();
        emit SocketMsg(byte, byte.size());
    }

}

void SocketClass::WriteToSocket(const QByteArray &byte)
{
    if(socket_state_ == OPEN&&my_socket_)
    {
        my_socket_->write(byte);
    }
}

bool SocketClass::OpenSocket()
{
    if(0 == my_socket_)
    {
        my_socket_ = new QTcpSocket();
    }
    my_socket_->connectToHost("115.159.127.79", 5678);
    if(my_socket_->waitForConnected(1000))
    {
        socket_state_ = OPEN;
        connect(my_socket_, SIGNAL(readyRead()), this, SLOT(ReadFromSocket()));
        qDebug()<<"Socket connect success";
        my_socket_->write("0/0/");
        return true;
    }
    else
    {
        qDebug()<<"Socket connect failed";
    }

    return false;


}

bool SocketClass::CloseSocket()
{
    if(my_socket_ && socket_state_ == OPEN)
    {
        my_socket_->close();
        if(my_socket_->waitForDisconnected())
        {
            socket_state_ = CLOSE;
            return true;
        }
    }

    return false;
}

void SocketClass::ReleaseSocket()
{
    if(my_socket_)
    {
        delete my_socket_;
    }
    my_socket_ = 0;
}





