#include "moudle.h"

Moudle::Moudle(QObject *parent) :
    QObject(parent),
    count_(0)
{
}


int Moudle::time_cycle_(100);
void Moudle::HandleSerialMsg(const QByteArray &)
{
    count_ = 0;
    if(0!=coor_)
    {
        qint8  id;
        GetID(id);
        coor_->EmitStatus(id, true);
        //qDebug() << id;
    }
}

void Moudle::set_coor(Coordinator  *coor)
{
    coor_ = coor;
}

void Moudle::set_serial_service(SerialService *service)
{
    serial_service_ = service;
}

void Moudle::set_socket_service(SocketClass *service)
{
    socket_serivce_ = service;
}

SerialService* Moudle::get_serial_service()
{
    return serial_service_;
}

SocketClass* Moudle::get_socket_service()
{
    return socket_serivce_;
}

void Moudle::CheckStatus()
{
    if(count_>MOUDLEDEADLINE)
    {
        if(0!=coor_)
        {
            qint8  id;
            GetID(id);
            coor_->EmitStatus(id, false);
        }
    }
    else
    {
        count_+=time_cycle_;
    }
}


unsigned char Moudle::Varify (unsigned char  *date, unsigned short len )
{
    unsigned char num = 0;
    unsigned short i;
    for (i = 0; i < len; i++)
    {
        num+= date[i];
    }
    return num;
}

void Moudle::set_time_cycle(int &timecycle)
{
    time_cycle_ = timecycle;
}

void Moudle::HandleWrongRequest(QByteArray &wrong_msg)
{
    socket_serivce_->WriteToSocket(wrong_msg);
}

