#include "replays.h"

Replays::Replays()
{
    confirm_counter_ = 0;
    for(int i = 0;i<4;i++)
        pre_status_[i] = replays_status_[i] = false;
    confirm_timer_ = new QTimer();
    connect(confirm_timer_,SIGNAL(timeout()),this,SLOT(ConfirmOpen()));
}

QByteArray Replays::msg_("\x40\x07\x01\x0a\x00\x00\xFF", 7);


void Replays::WriteToSerial(const QByteArray &byte)
{
    SerialService *serivce = Moudle::get_serial_service();
    if(0!=serivce)
    {
        serivce->WriteToSerial(byte);
    }
}

qint8 Replays::GetID(qint8 &id)
{
    id=0x0a;
}

void Replays::HandleSerialMsg(const QByteArray &byte)//处理收到的消息
{
    Moudle::HandleSerialMsg(byte);
    int len;
    len = byte[1];
    if (len == 8 && byte[4] != 0xaa)
    {
        if ((char)(0x08 & byte[5])== (char)0x08 && (char)(0x08 & byte[6]) == (char)0x08)
        {
            pre_status_[0] = true;
        }

        if ((char)(0x08 & byte[5])== (char)0x08 && (char)(0x08 & byte[6]) == (char)0x00)
        {
            pre_status_[0] = false;
        }

        if ((char)(0x04 & byte[5])== (char)0x04 && (char)(0x04 & byte[6]) == (char)0x04)
        {
            pre_status_[1] = true;
        }

        if ((char)(0x04 & byte[5])== (char)0x04 && (char)(0x04 & byte[6]) == (char)0x00)
        {
            pre_status_[1] = false;
        }

        if ((char)(0x02 & byte[5])== (char)0x02 && (char)(0x02 & byte[6]) == (char)0x02) {
            pre_status_[2] = true;
        }

        if ((char)(0x02 & byte[5])== (char)0x02 && (char)(0x02 & byte[6]) == (char)0x00) {
            pre_status_[2] = false;
        }

        if ((char)(0x01 & byte[5])== (char)0x01 && (char)(0x01 & byte[6]) == (char)0x01) {
            pre_status_[3] = true;
        }

        if ((char)(0x01 & byte[5])== (char)0x01 && (char)(0x01 & byte[6]) == (char)0x00) {
            pre_status_[3] = false;
        }
    }
}

void Replays::SendMsg(qint8 &cmd, qint8 &content)//发送消息
{
    if (1 > cmd || 5 < cmd)
    {
        return ;
    }
    if(!(content == 0||content == 1)) return;

    if(cmd == 0x01)
    {
        msg_[4] = 0x08;
        if(content)
        {
            msg_[5] = 0x08;
            replays_status_[0] = true;
        }
        else
        {
            msg_[5] = 0x00;
            replays_status_[0] = false;
        }
    }
    else if(cmd == 0x02)
    {
        msg_[4] = 0x04;
        if(content)
        {
            msg_[5] = 0x04;
            replays_status_[1] = true;
        }
        else
        {
            msg_[5] = 0x00;
            replays_status_[1] = false;
        }
    }
    else if(cmd == 0x03)
    {
        msg_[4] = 0x02;
        if(content)
        {
            msg_[5] = 0x02;
            replays_status_[2] = true;
        }
        else
        {
            msg_[5] = 0x00;
            replays_status_[2] = false;
        }
    }
    else if(cmd == 0x04)
    {
        msg_[4] = 0x01;
        if(content)
        {
            msg_[5] = 0x01;
            replays_status_[3] = true;
        }
        else
        {
            msg_[5] = 0x00;
            replays_status_[3] = false;
        }
    }

    unsigned char var;
    char *str;
    str = msg_.data();
    var = Moudle::Varify((unsigned char *)str, 6);
    msg_[6] = var;
    WriteToSerial(msg_);

    WriteToSerial(msg_);

    //confirm_counter_ = 0;
    //confirm_timer_->start(500);
}

QByteArray Replays::GetSensorInfo()
{

}

void Replays::ConfirmOpen()
{
    if(confirm_counter_>=5)
    {
        confirm_counter_ = 0;
        for(int i = 0;i<4;i++)
        {
            if(replays_status_[i] != pre_status_[i])
            {
                QString s("0/4/");
                QByteArray wmsg = s.toAscii() + QByteArray("\x0a");
                wmsg.append('1'+i);
                wmsg.append("\x00");
                Moudle::HandleWrongRequest(wmsg);
                replays_status_[i] = pre_status_[i];
            }

        }
        confirm_timer_->stop();
        return;
    }
    for(int i = 0;i<4;i++)
    {
        if(pre_status_[i] != replays_status_[i])
        {
            WriteToSerial(msg_);
            qDebug()<<"undone";
            confirm_counter_++;
        }
    }


    /*
    if(pre_status_[0]!=replays_status_[0]
            ||pre_status_[1]!=replays_status_[1]
            ||pre_status_[2]!=replays_status_[2]
            ||pre_status_[3]!=replays_status_[3])
    {
        WriteToSerial(msg_);
        qDebug()<<"undone";
    }
    else
    {
        confirm_timer_->stop();
        qDebug()<<"done";
    }
    */
}


void Replays::HandleSocketMsg(qint8 &cmd, qint8 &content)
{
    SendMsg(cmd, content);
    qDebug() << cmd << content << "===";
}
