#pragma once

#include <QtCore/QObject>

class TestConnectionReciever : public QObject
{
    Q_OBJECT
public:
    TestConnectionReciever(QObject* parent = nullptr) : QObject(parent) {};
    ~TestConnectionReciever() {};

public slots:
    void testSlotSimple(QStringList parameters);
    void testSlotWithMonitor(QStringList parameters);
    void onConnectSender(int state, QString object_name, QString connection_key);
    void onDisonnectSender(int state, QString object_name, QString connection_key);
};
