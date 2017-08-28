#ifndef SERIALCLASS_H
#define SERIALCLASS_H

#include <QObject>
#include <posix_qextserialport.h>
#include "serialservice.h"

class SerialClass : public QObject, public SerialService
{
    Q_OBJECT
public:
    explicit SerialClass(QObject *parent = 0);
    static SerialService *GetService();
private://From SerialService
    qint64 ReadFromSerial(QByteArray &byte);
    void WriteToSerial(const QByteArray &byte);
    bool OpenCom();
    bool CloseCom();
    void ReleaseSerial();
    
signals:
    
public slots:

private:
    Posix_QextSerialPort *my_com_;
    static SerialService *serial_service_;
    enum{OPEN, CLOSE};
    int com_state_;
};

#endif // SERIALCLASS_H
