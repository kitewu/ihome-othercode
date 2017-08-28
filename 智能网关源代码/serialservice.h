#ifndef SERIALSERVICE_H
#define SERIALSERVICE_H
#include <QByteArray>

class SerialService
{
public:
    virtual qint64 ReadFromSerial(QByteArray &byte) = 0;
    virtual void WriteToSerial(const QByteArray &byte) = 0;
    virtual bool OpenCom() = 0;
    virtual bool CloseCom() = 0;
    virtual void ReleaseSerial() = 0;
};

#endif // SERIALSERVICE_H
