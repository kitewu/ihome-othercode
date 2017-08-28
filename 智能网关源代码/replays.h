#ifndef REPLAYS_H
#define REPLAYS_H

#include <QByteArray>
#include <QDebug>
#include <moudle.h>
#include <QTimer>
#include <serialservice.h>



class Replays :public Moudle
{
    Q_OBJECT
public:
    explicit Replays();

    void WriteToSerial(const QByteArray &byte);

    qint8 GetID(qint8 &id);

    void HandleSerialMsg(const QByteArray &byte);

    void HandleSocketMsg(qint8 &cmd, qint8 &content);

    QByteArray GetSensorInfo();

signals:

public slots:
    void ConfirmOpen();

private:
    void SendMsg(qint8 &cmd, qint8 &content);

private:
    static QByteArray msg_;
    bool replays_status_[4];
    bool pre_status_[4];
    QTimer *confirm_timer_;
    int confirm_counter_;

};

#endif // REPLAYS_H
