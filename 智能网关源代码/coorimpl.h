#ifndef COORIMPL_H
#define COORIMPL_H

#include <QObject>
#include "abstracemoudle.h"

class CoorImpl : public QObject, public Coordinator
{
    Q_OBJECT
public:
    explicit CoorImpl(QObject *parent = 0);

    void EmitStatus(qint8 &id, bool status);

    static Coordinator* GetCoordinator();
    
signals:
    void Status(qint8 id, bool status);
    
public slots:

private:
    static Coordinator *coor_;
    
};

#endif // COORIMPL_H
