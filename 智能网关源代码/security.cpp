#include "security.h"
#include "socketclass.h"

Security::Security()
{
    status = CLOSE;
    people_status = false;
}

void Security::WriteToSerial(const QByteArray &)
{

}

qint8 Security::GetID(qint8 &id)
{
    return id = 0x05;
}

void Security::HandleSerialMsg(const QByteArray &byte)
{
    Moudle::HandleSerialMsg(byte);

    people_status = byte[5] == 0x01?true:false;

    if(status == OPEN&&people_status)
    {
        qDebug() << "People coming in";
        SocketClass *service = SocketClass::GetSocket();
        service->WriteToSocket(GetSensorInfo());
    }
}

void Security::HandleSocketMsg(qint8 &, qint8 &content)
{
    status = content?OPEN:CLOSE;
}

QByteArray Security::GetSensorInfo()
{
    return QString("0/4/3").toAscii();

}
