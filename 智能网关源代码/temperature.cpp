#include "temperature.h"

Temperature::Temperature():
    temperature_(0),
    light_(0),
    humidity_(0),
    time_count_(0)
{
}


void Temperature::WriteToSerial(const QByteArray &)
{

}

qint8 Temperature::GetID(qint8 &id)
{
    id=0x02;
}

void Temperature::HandleSerialMsg(const QByteArray &byte)
{
    unsigned char adc_value[2];

    Moudle::HandleSerialMsg(byte);

    if (0x01 == byte[4])
    {
        temperature_ = (byte[5] << 8) + byte[6];//计算温度值公式
        humidity_ = (byte[7] << 8) + byte[8];//计算湿度值公式

        adc_value[0] = byte[10];
        adc_value[1] = byte[9];
        adc_value[0] = adc_value[0] >> 2;
        light_ = (adc_value[1]*256 + adc_value[0]) * 3.3 / 8192;
        light_ = light_ / 4;
        light_ = light_ * 913;
    }
    if(time_count_==9)
    {
        QByteArray json_msg = GetSensorInfo();
        SocketClass* service = get_socket_service();
        service->WriteToSocket(json_msg);
    }
    time_count_ = (time_count_+1)%10;
}

void Temperature::HandleSocketMsg(qint8 &, qint8 &)
{

}

void Temperature::SendMsg(qint8 &, qint8 &)
{

}

QByteArray Temperature::GetSensorInfo()
{
    QString info;
    info+="0/2/";
    info+=QString::number(temperature_)+";";
    info+=QString::number(humidity_)+";";
    info+=QString::number(light_,'f',1);
    return info.toAscii();
}
