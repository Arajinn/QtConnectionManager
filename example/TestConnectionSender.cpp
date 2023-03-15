#include "TestConnectionSender.h"

#include <QtCore/QDebug>

void TestConnectionSender::onConnectReciever(int state, QString object_name, QString connection_key)
{
    const QString thisObjName = objectName();
    qDebug() << "Established connection:" << connection_key;
    qDebug() << "From:" << thisObjName;
    qDebug() << "To:" << object_name;
    qDebug() << "";
}

void TestConnectionSender::onDisonnectReciever(int state, QString object_name, QString connection_key)
{
    const QString thisObjName = objectName();
    qDebug() << "Remove connection:" << connection_key;
    qDebug() << "From:" << thisObjName;
    qDebug() << "To:" << object_name;
    qDebug() << "";
}