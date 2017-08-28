#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    c = new Clock(ui->ClockWidget);
    c->show();

    this->showFullScreen();

    moudle_set_ = new MoudleSet();


    //InitCamera();
}

MainWindow::~MainWindow()
{

    deviceUninit();
    delete ui;
}

void MainWindow::InitCamera()
{
    pic_write_ = false;
    camera_start_ = false;
    camera_timer_ = new QTimer();
    connect(camera_timer_,SIGNAL(timeout()), this, SLOT(UpdateCamera()));
    deviceOpen();
    deviceInit();
}

void MainWindow::UpdateCamera()
{
    unsigned char image_buf[1536000+54];

    frameRead(image_buf);
    //qDebug()<<"image_buf"<<*image_buf<<endl;
    SendPicture(image_buf);
    this->qimage_ = QImage::fromData(image_buf,800*480*4+54,NULL);


    pixmap_ = QPixmap::fromImage(this->qimage_, 0);
    ui->labelvideo->setPixmap(this->pixmap_);

    if (pic_write_)
    {
        FILE* bmp_f = fopen("a.bmp", "w+");
        fwrite(image_buf, 1, 800*480*4+54, bmp_f);  //debug by liaoxp 2013-11-28
        fclose(bmp_f);
        pic_write_ = false;
        qDebug()<<"take photoshop............................."<<endl;
    }
}


void MainWindow::on_OPEN_clicked()
{
    /*
    if(camera_start_)
    {
        camera_start_ = false;
        camera_timer_->stop();
    }
    else
    {
        camera_start_ = true;
        captureStart();
        camera_timer_->start(300);

    }*/
    //SerialService *service = SerialClass::GetService();
    //QByteArray m("\x40\x07\x01\x10\x01\x01\x00", 7);
    //service->WriteToSerial(m);

}

void MainWindow::on_CLOSE_clicked()
{
    //camera_start_ = false;
    //camera_timer_->stop();
    //qint8 msg = 2;
    //replay_moudle_->SendMsg(msg);
    /*QByteArray m("\x40\x07\x01\x0f\x04\x00\x00", 7);
    char *str = m.data();
    m[6] = Varify((unsigned char *)str, 6);
    SerialService *service = SerialClass::GetService();
    service->WriteToSerial(m);*/

}

void MainWindow::SendPicture(unsigned char *pic)
{
    /*
    qDebug()<<"sending pic";
    char *t = (char *)pic;
    QString temp = QString(t);
    using namespace std;
    Json::Value root;
    root["ID"] = "10";
    root["PIC"] = temp.toStdString();

    std::string out = root.toStyledString();
    const char* status = out.c_str();
    my_socket_service_->WriteToSocket(QByteArray((char *)status));
    */
}
