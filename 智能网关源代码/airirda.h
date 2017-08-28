#ifndef AIRIRDA_H
#define AIRIRDA_H
#include <moudle.h>

class AirIrDA : public Moudle
{
public:
    AirIrDA();

    void WriteToSerial(const QByteArray &byte);

    qint8 GetID(qint8 &id);

    void HandleSerialMsg(const QByteArray &byte);

    void HandleSocketMsg(qint8 &cmd, qint8 &content);

    QByteArray GetSensorInfo();

private:
    void SendMsg(qint8 &cmd, qint8 &content);
private:
    static QByteArray msg_;
    static qint8 current_temp_;
};

#endif // AIRIRDA_H
