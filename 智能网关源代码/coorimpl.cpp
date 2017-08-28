#include "coorimpl.h"

CoorImpl::CoorImpl(QObject *parent) :
    QObject(parent)
{
}

Coordinator *CoorImpl::coor_(0);

void CoorImpl::EmitStatus(qint8 &id, bool status)
{
    emit Status(id, status);
}

Coordinator *CoorImpl::GetCoordinator()
{
    if(coor_ == 0) coor_ = new CoorImpl();
    return coor_;
}
