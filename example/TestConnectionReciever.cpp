#include "TestConnectionReciever.h"

#include <QtCore/QDebug>

void TestConnectionReciever::testSlotSimple(QStringList parameters)
{
    qDebug() << objectName() << "get message:" << parameters.join(" ");
    qDebug() << "";
}

void TestConnectionReciever::testSlotWithMonitor(QStringList parameters)
{
    qDebug() << objectName() << "get message:" << parameters.join(" ");
    qDebug() << "";
}

void TestConnectionReciever::onConnectSender(int state, QString object_name, QString connection_key)
{
    const QString thisObjName = objectName();
    qDebug() << "Established connection:" << connection_key;
    qDebug() << "To:" << thisObjName;
    qDebug() << "From:" << object_name;
    qDebug() << "";
}

void TestConnectionReciever::onDisonnectSender(int state, QString object_name, QString connection_key)
{
    const QString thisObjName = objectName();
    qDebug() << "Remove connection:" << connection_key;
    qDebug() << "To:" << thisObjName;
    qDebug() << "From:" << object_name;
    qDebug() << "";
}