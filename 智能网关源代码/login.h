#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QDebug>
#include "WidgetKeyboard.hpp"


namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT
    
public:
    explicit Login(QWidget *parent = 0);
    ~Login();
protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void on_Login_Btn_clicked();

    void on_UserName_Edit_textEdited(const QString &arg1);

private:
    Ui::Login *ui;
    WidgetKeyboard *keyboard;
};

#endif // LOGIN_H
