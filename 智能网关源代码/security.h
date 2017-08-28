#ifndef SECURITY_H
#define SECURITY_H
#include "moudle.h"

class Security : public Moudle
{
public:
    Security();

    void WriteToSerial(const QByteArray &byte);

    qint8 GetID(qint8 &id);

    void HandleSerialMsg(const QByteArray &byte);

    void HandleSocketMsg(qint8 &cmd, qint8 &content);

    QByteArray GetSensorInfo();

private:
    enum{OPEN, CLOSE}status;
    bool people_status;
};

#endif // SECURITY_H
