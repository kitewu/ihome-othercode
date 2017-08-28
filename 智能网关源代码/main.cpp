#include <QtGui/QApplication>
#include "mainwindow.h"
#include "clock.h"
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    Login l;
    l.show();
    return a.exec();
}
