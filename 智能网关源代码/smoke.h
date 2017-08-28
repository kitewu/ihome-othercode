#ifndef SMOKE_H
#define SMOKE_H
#include <QDebug>
#include "moudle.h"

class Smoke : public Moudle
{
public:
    Smoke();
    void WriteToSerial(const QByteArray &byte);

    qint8 GetID(qint8 &id);

    void HandleSerialMsg(const QByteArray &byte);

    void HandleSocketMsg(qint8 &, qint8 &);

    QByteArray GetSensorInfo();

private:
    void SendMsg(qint8 &cmd, qint8 &content);

private:
    bool smoke_state_;
};

#endif // SMOKE_H
