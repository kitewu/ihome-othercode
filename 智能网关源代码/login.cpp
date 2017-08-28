#include "login.h"
#include "ui_login.h"
#include "mainwindow.h"

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    this->showFullScreen();

    ui->UserName_Edit->installEventFilter(this);
    ui->Passwd_Edit->installEventFilter(this);

    keyboard = new WidgetKeyboard(this);
}

Login::~Login()
{
    delete ui;
}

void Login::on_Login_Btn_clicked()
{
    keyboard->close();
    this->close();

}

void Login::on_UserName_Edit_textEdited(const QString &arg1)
{

}

bool Login::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->UserName_Edit)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            //qDebug() << "UNE_Pressed";
            keyboard->show();
        }
    }

    if(watched == ui->Passwd_Edit)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            //qDebug() << "PassWd_Pressed";
            keyboard->show();
        }
    }

    return QWidget::eventFilter(watched, event);
}
