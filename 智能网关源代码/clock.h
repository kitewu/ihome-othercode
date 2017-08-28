#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QTime>

class Clock : public QWidget
{
    Q_OBJECT
public:
    explicit Clock(QWidget *parent = 0);
    
signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
private:
    QTimer *update_timer_;
    
};

#endif // CLOCK_H
