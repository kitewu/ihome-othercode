#ifndef ABSTRACEMOUDLE_H
#define ABSTRACEMOUDLE_H

#include <QByteArray>
#include <serialservice.h>
#include <socketclass.h>



class Coordinator
{
public:
    virtual void EmitStatus(qint8 &id, bool status) = 0;
};

class AbstractMoudle
{
public:
    virtual qint8 GetID(qint8 &id) = 0;
    virtual void HandleSerialMsg(const QByteArray &msg) = 0;
    virtual void HandleSocketMsg(qint8 &cmd, qint8 &content) = 0;
    virtual void set_serial_service(SerialService *service) = 0;
    virtual void set_socket_service(SocketClass *service) = 0;
    virtual void set_coor(Coordinator *coor) = 0;
    virtual QByteArray GetSensorInfo() = 0;
    virtual void CheckStatus() = 0;
    virtual void set_time_cycle(int &cycle) = 0;
    virtual void HandleWrongRequest(QByteArray &wrong_msg) = 0;
protected:
    virtual SerialService * get_serial_service() = 0;
};

#endif // ABSTRACEMOUDLE_H
