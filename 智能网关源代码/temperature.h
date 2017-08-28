#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <QByteArray>
#include <QDebug>
#include <moudle.h>

class Temperature : public Moudle
{
    Q_OBJECT
public:
    Temperature();

    void WriteToSerial(const QByteArray &byte);

    qint8 GetID(qint8 &id);

    void HandleSerialMsg(const QByteArray &byte);

    void HandleSocketMsg(qint8 &cmd, qint8 &content);

    QByteArray GetSensorInfo();

private:
    void SendMsg(qint8 &cmd, qint8 &content);

private:
    int temperature_;
    int humidity_;
    float light_;
    int time_count_;
};

#endif // TEMPERATURE_H
