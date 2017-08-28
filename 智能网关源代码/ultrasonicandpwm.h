#ifndef ULTRASONICANDPWM_H
#define ULTRASONICANDPWM_H
#include <QDebug>
#include <QTimer>
#include "moudle.h"

class UltrasonicAndPwm : public Moudle
{
    Q_OBJECT
public:
    UltrasonicAndPwm();

    void WriteToSerial(const QByteArray &byte);

    qint8 GetID(qint8 &id);

    void HandleSerialMsg(const QByteArray &byte);

    void HandleSocketMsg(qint8 &cmd, qint8 &content);

    QByteArray GetSensorInfo();

private slots:
    void ChangeLightState();

private:
    void DealWithDisance(unsigned short &value);

    void SendMsg(qint8 &cmd, qint8 &content);

private:
    static QByteArray msg_;
    int current_distance_;
    int current_light_;
    bool open_;
    qint8 light_status_;
    QTimer *light_timer_;
};

#endif // ULTRASONICANDPWM_H
