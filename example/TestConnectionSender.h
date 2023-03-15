#pragma once

#include <QtCore/QObject>

class TestConnectionSender : public QObject
{
    Q_OBJECT
public:
    TestConnectionSender(QObject* parent = nullptr) : QObject(parent) {};
    ~TestConnectionSender() {};

    Q_INVOKABLE void onConnectReciever(int state, QString object_name, QString connection_key);
    Q_INVOKABLE void onDisonnectReciever(int state, QString object_name, QString connection_key);
signals:
    void testEventSimple(QStringList parameters);
    void testEventWithMonitor(QStringList parameters);
    
};
