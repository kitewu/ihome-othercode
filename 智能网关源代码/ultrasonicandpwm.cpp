#include "ultrasonicandpwm.h"

UltrasonicAndPwm::UltrasonicAndPwm():
    current_distance_(0)
{
    light_timer_ = new QTimer();
    connect(light_timer_, SIGNAL(timeout()), this, SLOT(ChangeLightState()));
}

QByteArray UltrasonicAndPwm::msg_("\x40\x06\x01\x09\x00\xFF", 6);

//获得超声波的距离数据
void UltrasonicAndPwm::HandleSerialMsg(const QByteArray &byte)
{
    unsigned short value = 0;
    Moudle::HandleSerialMsg(byte);

    if(byte[3] == 0x08)//超声波
    {
        value = value + byte[5];
        value = value*256;
        value = value + ((unsigned short)byte[6]&0x00ff);
        if(value  == 0) return;
        //qDebug() << "Current : " << current_distance_ << "Rece : " << value;
        if(open_)
        {
            DealWithDisance(value);
            current_distance_ = value;
        }


        else
            current_distance_ = value;
    }

    if(byte[3] == 0x09)//PWM调光
    {
        current_light_ = (int)byte[5];
    }
}

//第一位是命令，第二位是内容
void UltrasonicAndPwm::SendMsg(qint8 &, qint8 &content)
{
    msg_[4] = content;
    unsigned char var;
    char *str;
    str = msg_.data();
    var = Moudle::Varify((unsigned char *)str, 5);
    msg_[5] = var;

    WriteToSerial(msg_);
}

void UltrasonicAndPwm::DealWithDisance(unsigned short &value)
{
    if(current_distance_ - value >= 300)
    {
        qDebug() << "People Awake";
        if(!light_timer_->isActive())
        {
            light_timer_->start(3000);
            light_status_ = 0x01;
        }
    }
}

void UltrasonicAndPwm::ChangeLightState()
{
    qDebug() << "Change light status";
    qint8 cmd = 0x00;
    if(light_status_ <= 0x09)
    {

        SendMsg(cmd, light_status_);
        light_status_++;
    }
    else if(light_status_ == 0x0f)
    {
        light_status_ = 0x00;
        SendMsg(cmd, light_status_);
        light_timer_->stop();
    }
    else
    {
        light_status_++;
    }

}

void UltrasonicAndPwm::HandleSocketMsg(qint8 &, qint8 &content)
{
    qDebug() << " Receive msg : " << content;
    if(content) open_ = true;
    else open_ = false;
}

void UltrasonicAndPwm::WriteToSerial(const QByteArray &byte)
{
    SerialService *serivce = Moudle::get_serial_service();
    if(0!=serivce)
    {
        serivce->WriteToSerial(byte);
    }
}

qint8 UltrasonicAndPwm::GetID(qint8 &id)
{
    id = 0x08;
}

QByteArray UltrasonicAndPwm::GetSensorInfo()
{
    return NULL;
}
