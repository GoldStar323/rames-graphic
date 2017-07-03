#include "ramasset.h"
#ifdef QT_DEBUG
#include <QtDebug>
#endif

RAMAsset::RAMAsset(DBInterface *db, QString n, QString sn, RAMStatus *st,int stageId, bool updateDb, QString c, int i, QObject *parent) : QObject(parent)
{
    id = i;
    name = n;
    status = st;
    connect(status,SIGNAL(statusRemoved(RAMStatus*)),this,SLOT(statusDeleted(RAMStatus*)));
    shortName = sn;
    comment = c;

    dbi = db;
    if (updateDb)
    {
        id = dbi->addAsset(name,shortName,status->getId(),stageId,comment);
    }
}

RAMAsset::~RAMAsset()
{

}

int RAMAsset::getId()
{
    return id;
}

QString RAMAsset::getComment()
{
    return comment;
}

QList<RAMShot *> RAMAsset::getAssignments()
{
    return assignments;
}

void RAMAsset::assign(RAMShot *shot, bool updateDb)
{
    assignments << shot;
    connect(shot,SIGNAL(shotRemoved(RAMShot*)),this,SLOT(shotDeleted(RAMShot*)));
    if (updateDb)
    {
        dbi->assignAsset(id,shot->getId());
    }
    emit assetAssigned(shot);
}

void RAMAsset::unAssign(RAMShot *shot, bool updateDb)
{
    assignments.removeAll(shot);
    emit assetUnAssigned(shot);
}

void RAMAsset::setName(QString n,bool updateDb)
{
    name = n;
    emit nameChanged(n);
}

void RAMAsset::setShortName(QString sn,bool updateDb)
{
    shortName = sn;
    emit shortNameChanged(sn);
}

void RAMAsset::setComment(QString c,bool updateDb)
{
    comment = c;
    emit commentChanged(c);
}

void RAMAsset::setStatus(RAMStatus *s, bool updateDb)
{
    status = s;
    if (updateDb) dbi->setAssetStatus(s->getId(),id);
    emit statusChanged(this,s);
    emit statusChanged(s);
}

void RAMAsset::update()
{
    dbi->updateAsset(id,name,shortName,comment);
}

void RAMAsset::remove(bool updateDB)
{
    emit assetRemoved(this);
}

void RAMAsset::statusDeleted(RAMStatus *s)
{
    setStatus(0,false);
}

void RAMAsset::shotDeleted(RAMShot *s)
{
    unAssign(s,false);
}

RAMStatus* RAMAsset::getStatus()
{
    return status;
}

QString RAMAsset::getName()
{
    return name;
}

QString RAMAsset::getShortName()
{
    return shortName;
}

bool RAMAsset::operator==(RAMAsset a)
{
    return a.getId() == id;
}

bool RAMAsset::operator==(RAMAsset *a)
{
    return a->getId() == id;
}
