#ifndef CURTAIN_H
#define CURTAIN_H

#include "moudle.h"

class Curtain : public Moudle
{
public:
    Curtain();

    //向串口写数据
    void WriteToSerial(const QByteArray &byte);

    //获取节点ID
    qint8 GetID(qint8 &id);

    //处理串口消息
    void HandleSerialMsg(const QByteArray &byte);

    //处理服务器消息
    void HandleSocketMsg(qint8 & msg, qint8 &content);

    //获取当前传感器的信息
    QByteArray GetSensorInfo();

private:
    void SendMsg(qint8 &, qint8 &content);

private:
    bool open;
    static QByteArray msg_;


};

#endif // CURTAIN_H
