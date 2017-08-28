#ifndef MOUDLE_H
#define MOUDLE_H

#include <QObject>
#include <QByteArray>
#include <QDebug>
#include "abstracemoudle.h"

#define MOUDLEDEADLINE 5000

class Moudle : public QObject, public AbstractMoudle
{
    Q_OBJECT
public:
    explicit Moudle(QObject *parent = 0);

    virtual void HandleSerialMsg(const QByteArray &msg);

    virtual void HandleSocketMsg(qint8 &cmd, qint8 &content) = 0;

    virtual void set_coor(Coordinator *coor);

    virtual void set_serial_service(SerialService *service);

    virtual void set_socket_service(SocketClass *service);

    virtual void CheckStatus();

    virtual qint8 GetID(qint8 &id) = 0;

    virtual QByteArray GetSensorInfo() = 0;

    virtual void set_time_cycle(int &timecycle);

    virtual void HandleWrongRequest(QByteArray &wrong_msg);
signals:

    
public slots:

protected:
    virtual SerialService *get_serial_service();
    virtual SocketClass *get_socket_service();

    virtual

    unsigned char Varify (unsigned char  *date, unsigned short len);
private:
    SerialService *serial_service_;
    SocketClass *socket_serivce_;
    Coordinator *coor_;
    int count_;
    static int time_cycle_;
    
};

#endif // MOUDLE_H
