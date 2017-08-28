#include "curtain.h"
#include "serialclass.h"
#include "serialservice.h"

Curtain::Curtain() : open(false)
{
}

QByteArray Curtain::msg_("\x40\x07\x01\x10\x01\x01\x00", 7);

void Curtain::WriteToSerial(const QByteArray &byte)
{
    SerialService *service;
    service = SerialClass::GetService();
    service->WriteToSerial(byte);
}

//获取节点ID
qint8 Curtain::GetID(qint8 &id)
{
    return id = 0x10;
}

//处理串口消息
void Curtain::HandleSerialMsg(const QByteArray &byte)
{
    Moudle::HandleSerialMsg(byte);
}

//处理服务器消息
void Curtain::HandleSocketMsg(qint8 &msg, qint8 &content)
{
    SendMsg(msg, content);
}

//获取当前传感器的信息
QByteArray Curtain::GetSensorInfo()
{

}

void Curtain::SendMsg(qint8 &, qint8 &content)
{
    msg_[4] = content==0x01?0x01:0x00;

    unsigned char var;
    char *str;
    str = msg_.data();
    var = Moudle::Varify((unsigned char *)str, 6);
    msg_[6] = var;
    qDebug() << "Curtain : " << msg_;
    WriteToSerial(msg_);
}
