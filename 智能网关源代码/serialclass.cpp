#include "serialclass.h"

SerialClass::SerialClass(QObject *parent) :
    QObject(parent),
    my_com_(0),
    com_state_(CLOSE)
{

}

SerialService* SerialClass::serial_service_(0);

SerialService *SerialClass::GetService()
{
    if(0 == serial_service_)
    {
        serial_service_ = new SerialClass;
    }
    return serial_service_;
}

qint64 SerialClass::ReadFromSerial(QByteArray &byte)
{
    if(my_com_&&OPEN == com_state_)
    {
        byte = my_com_->readAll();
        return byte.length();
    }
    return -1;
}

void SerialClass::WriteToSerial(const QByteArray &byte)
{
    if(my_com_&&OPEN == com_state_)
    {
        my_com_->write(byte);
    }
}




bool SerialClass::OpenCom()
{
    if(my_com_)
    {
        CloseCom();
    }
    else
    {
        QString com_name_ = "/dev/ttySAC1";
        my_com_ = new Posix_QextSerialPort(com_name_, QextSerialBase::Polling);
    }

    my_com_->open(QIODevice::ReadWrite);
    if(my_com_->isOpen())
    {
        com_state_ = OPEN;
        my_com_->setBaudRate(BAUD115200);
        my_com_->setDataBits(DATA_8);
        my_com_->setParity(PAR_NONE);
        my_com_->setStopBits(STOP_1);
        my_com_->setFlowControl(FLOW_OFF);
        my_com_->setTimeout(50);
    }
    else
    {
        com_state_ = false;
    }



    return true;
}

bool SerialClass::CloseCom()
{
    if(my_com_&&CLOSE!=com_state_)
    {
        my_com_->close();
    }
    com_state_ = CLOSE;

    return com_state_;
}

void SerialClass::ReleaseSerial()
{
    if(my_com_)
    {
        CloseCom();
        delete my_com_;
        my_com_ = 0;
    }
}
