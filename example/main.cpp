#include <QtCore/QCoreApplication>

#include "../ConnectionManager/connectionmanager.h"
#include "TestConnectionSender.h"
#include "TestConnectionReciever.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ConnectionManager* manager = new ConnectionManager();

    const QString simple_connection_key = "simple_connection";
    const QString monitor_connection_key = "monitor_connection";

    TestConnectionSender* sender = new TestConnectionSender();
    sender->setObjectName("TestSenderObject");

    {
        manager->addEvent(sender, simple_connection_key, SIGNAL(testEventSimple(QStringList)));
    }

    {
        ConnectionManager::options options;
        options.on_connect_slot_name = SLOT(onConnectReciever(int, QString, QString));
        options.on_disconnect_slot_name = SLOT(onDisonnectReciever(int, QString, QString));
        options.connection_type = Qt::UniqueConnection;
        manager->addEvent(sender, monitor_connection_key, SIGNAL(testEventWithMonitor(QStringList)), options);
    }

    TestConnectionReciever* reciever = new TestConnectionReciever();
    reciever->setObjectName("TestRecieverObject");

    {
        manager->addSubscription(reciever, simple_connection_key, SLOT(testSlotSimple(QStringList)));
    }

    {
        ConnectionManager::options options;
        options.on_connect_slot_name = SLOT(onConnectSender(int, QString, QString));
        options.on_disconnect_slot_name = SLOT(onDisonnectSender(int, QString, QString));
        options.connection_type = Qt::UniqueConnection;
        manager->addSubscription(reciever, monitor_connection_key, SLOT(testSlotWithMonitor(QStringList)), options);
    }

    {
        const QString test_message = "test message";
        const QStringList parameters = test_message.split(" ");
        emit sender->testEventSimple(parameters);
    }

    reciever->deleteLater();

    return a.exec();
}
