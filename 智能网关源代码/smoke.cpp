#include "smoke.h"
#include "socketclass.h"

Smoke::Smoke():
    smoke_state_(false)
{
}


void Smoke::WriteToSerial(const QByteArray &byte)
{

}

qint8 Smoke::GetID(qint8 &id)
{
    id=0x04;
}

void Smoke::HandleSerialMsg(const QByteArray &byte)
{
    Moudle::HandleSerialMsg(byte);
    if(byte[5] == 0x01)
    {
        smoke_state_ = true;
        qDebug() << "Have Smoke";
        QByteArray json_msg = GetSensorInfo();
        SocketClass* service = get_socket_service();
        service->WriteToSocket(json_msg);
    }
    else
    {
        if(smoke_state_)
        {
            qDebug() << "Smoke disapper";
            smoke_state_ = false;
            QByteArray json_msg = GetSensorInfo();
            SocketClass* service = get_socket_service();
            service->WriteToSocket(json_msg);
        }
    }

}

void Smoke::SendMsg(qint8 &, qint8 &)
{

}

QByteArray Smoke::GetSensorInfo()
{
    QString info;
    info+="0/4/";
    if(smoke_state_) info+="1";
    else info+="0";
    return info.toAscii();
}
void Smoke::HandleSocketMsg(qint8 &, qint8 &)
{

}
