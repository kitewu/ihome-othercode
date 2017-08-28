#include "detectusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <QDebug>

DetectUsb::DetectUsb(QObject *parent) :
    QObject(parent)
{
    initSocketFd();
    findUsb = false;
    if(GetSocketFd() != -1) timer.start(100, this);
    isPlay_ = false;
}

int DetectUsb::socket_fd(0);

void DetectUsb::initSocketFd()
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if(hotplug_sock == -1)
    {
        printf("Error getting socket;%s\n", strerror(errno));
        return;
    }

    /*set receive buffersize*/
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    int flags=fcntl(hotplug_sock, F_GETFL,0);
    fcntl(hotplug_sock, F_SETFL, flags | O_NONBLOCK);


    retval = bind(hotplug_sock, (struct sockaddr*)&snl, sizeof(struct sockaddr_nl) );
    if(retval < 0)
    {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return;
    }
    socket_fd = hotplug_sock;

}


int DetectUsb::GetSocketFd()
{
    return socket_fd;
}

void DetectUsb::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == timer.timerId())
    {
        char buf[1024];
        recv(this->socket_fd, buf, sizeof(buf), 0);
        QString usbinfo = buf;

        QString pattern("(.*)/block/(.*)/(.*)");
        QRegExp rx(pattern);
        if(usbinfo.contains("add")&&usbinfo.contains(rx))
        {
            qDebug() << rx.cap(3);
            //emit FindUsbDevice(rx.cap(3));
            MountUsb(rx.cap(3));
        }
        else if(usbinfo.contains("remove")&&usbinfo.contains(rx))
        {
            qDebug() << rx.cap(3);
            //emit RemoveUsbDevice(rx.cap(3));
            UnMountUsb(rx.cap(3));
        }
    }
    else
        QObject::timerEvent(event);
}

void DetectUsb::MountUsb(QString name)
{
    findUsb = true;
    name = "/dev/" + name;
    QStringList arg;
    arg << name << "/usb";
    QProcess *p = new QProcess();
    p->start("mount", arg);
}

void DetectUsb::UnMountUsb(QString name)
{
    findUsb = false;
    name = "/dev/" + name;
    QStringList arg;
    arg << name << "/usb";
    QProcess *p = new QProcess();
    p->start("umount", arg);
}

QString DetectUsb::GetDir(QString s)
{
    if(!findUsb)
        return QString("0/1/") +hashCode + QString("/2||");
    else
    {
        QString str = QString("2||") + ReadDir(s);
        SendToServer(str);
    }

        return NULL;
}

QString DetectUsb::ReadDir(QString path)
{
    qDebug() << "Open Dir : " << path;
    QString filename = "";
    QString dirname = "";
    QDir dir(path);
    QStringList filter;
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(dir.entryInfoList(filter));
    for(int i = 0;i<fileInfo->count();i++)
    {
        QString name = fileInfo->at(i).fileName();
        qDebug() << " File name: "<< name;
        if(fileInfo->at(i).isDir())
        {
            if(name == QString("..") || name == QString(".")) continue;
            if(dirname == "") dirname = name + ";";
            else dirname = dirname + name + ";";
        }
        else
        {
            if(filename == "") filename = name + ";";
            else filename = filename + name + ";" ;
        }
    }
    QString p = dirname + "||" + filename;
    qDebug() << p;
    return p;
}

void DetectUsb::SetHashCode(QString code)
{
    hashCode = code;
}

void DetectUsb::PlayVideo(QString path)
{
    if(!findUsb)
        SendToServer(QString("0"));

    QFileInfo file(path);
    if(!file.isDir())
    {
        QProcess *p = new QProcess;
        QStringList arg;
        arg << file.absoluteFilePath();
        p->start("mplayer", arg);
        SendToServer(QString("1"));
    }
    else
    {
        SendToServer(QString("0"));
    }
}

void DetectUsb::SendToServer(QString content)
{
    QString msg = QString("0/1/") + hashCode + QString("/") + content;
    if(socketservice_)
    {
        socketservice_->WriteToSocket(msg.toAscii());
    }
}

void DetectUsb::CancelPlay()
{
    QProcess::execute("killall mplayer");

}

void DetectUsb::SetSocketService(SocketClass *service)
{
    this->socketservice_ = service;
}
