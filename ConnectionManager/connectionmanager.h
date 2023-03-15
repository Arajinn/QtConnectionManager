#pragma once

#include "connectionmanager_global.h"

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QMetaMethod>

class connected_object_info;

class CONNECTIONMANAGER_EXPORT ConnectionManager : public QObject
{
    Q_OBJECT
public:
    enum AddConnectionResult
    {
        Successful,
        NullFunctionPointer,
        SignalNotExist,
        SlotNotExist,
        PluginNotQtObject,
        EventAlreadyExist,
        OnConnectSlotNotExist,
        OnConnectSlotNotCompatible,
        OnDisconnectSlotNotExist,
        OnDisconnectSlotNotCompatible,
    };

    /*!
    * Class for remote QObject connection
    * Examples in ConnectionManager.pro
    */
    ConnectionManager();
    ~ConnectionManager();

    struct options
    {
        const char* on_connect_slot_name{ nullptr };
        const char* on_disconnect_slot_name{ nullptr };

        Qt::ConnectionType connection_type{ Qt::AutoConnection };
    };

    /*!
    * Add event to remote connections list
    *
    * Example sessionAgent->connectionManager()->addEvent(sender,"connection_key",SIGNAL(testSignal()));
    \param[in] QObject for connection
    \param[in] Connection id key (string constant, must be same for connection event and subscription)
    \param[in] Signature of signal (making by SIGNAL() macros)
    \param[in] Additional connection parameters: slot calls immediately after connect, slot calls immediately after disconnect, connection type
    \return Adding event result enum
    */
    AddConnectionResult addEvent(QObject* plugin, const QString& key, const char* signal_name, options input = options());

    /*!
    * Add subscription to remote connection list
    *
    * Example sessionAgent->connectionManager()->addSubscription(reciever,"connection_key",SLOT(testSlot()));
    \param[in] QObject for connection
    \param[in] Connection id key (string constant, must be same for connection event and subscription)
    \param[in] Signature of slot (making by SLOT() macros)
    \param[in] Additional connection parameters: slot calls immediately after connect, slot calls immediately after disconnect, connection type
    \return Adding event result enum
    */
    AddConnectionResult addSubscription(QObject* plugin, const QString& key, const char* slot_name, options input = options());

    /*!
    * Remove event from remote connections list
    *
    * Example sessionAgent->connectionManager()->removeEvent(sender,"connection_key");
    \param[in] QObject for connection
    \param[in] Connection id key
    */
    void removeEvent(QObject* plugin, const QString& key);

    /*!
    * Remove subscription from remote connections list
    *
    * Example sessionAgent->connectionManager()->removeSubscription(sender,"connection_key");
    \param[in] QObject for connection
    \param[in] Connection id key
    */
    void removeSubscription(QObject* plugin, const QString& key);

public:
    template <typename SignalFunc, typename MatchFunc>
    AddConnectionResult addEvent(QObject* plugin, const QString& key, SignalFunc signal, MatchFunc example,
        options input = options());

    template <typename SlotFunc, typename ExampleFunc>
    AddConnectionResult addSubscription(QObject* plugin, const QString& key, SlotFunc slot, ExampleFunc example, const char* slot_name,
        options input = options());

private:
    QVector<connected_object_info*> events;
    QVector<connected_object_info*> subscriptions;

    bool update_connections();

    AddConnectionResult addEventByIndex(QObject* plugin, const QString& key, const int& signal_index, const options& input);

    AddConnectionResult addEventByPointer(QObject* plugin, const QString& key, void** function, const options& input);

    bool addSubscriptionByIndex(QObject* plugin, const QString& key, const int& slot_index);

    void removeEventObjects(QObject* plugin, const QString& key);
    void removeSubscriptionObjects(QObject* plugin, const QString& key);

    bool makeConnection(connected_object_info* sender, const QMetaMethod& signal, connected_object_info* receiver, const QMetaMethod& slot);

    QPair<QString, QString> getSlotNamesByInputParams(options input);
private slots:
    void onDestroyedConnectionObject(QObject* obj);
};

template <typename SignalFunc, typename ExampleFunc>
ConnectionManager::AddConnectionResult ConnectionManager::addEvent(QObject* plugin, const QString& key, SignalFunc signal, ExampleFunc example,
    options input)
{
    CheckSignatureCompatible::CompilerSignatureCompatible(signal, example);

    auto function = reinterpret_cast<void**>(&signal);
    if (!function)
        return AddConnectionResult::NullFunctionPointer;

    return addEventByPointer(plugin, key, function, input);
}

template <typename SlotFunc, typename ExampleFunc>
ConnectionManager::AddConnectionResult ConnectionManager::addSubscription(QObject* plugin, const QString& key, SlotFunc slot, ExampleFunc example, const char* slot_name,
    options input)
{
    CheckSignatureCompatible::CompilerSignatureCompatible(example, slot);

    void** function = reinterpret_cast<void**>(&slot);
    if (!function)
        return AddConnectionResult::NullFunctionPointer;

    return addSubscription(plugin, key, slot_name, input);
}

class connected_object_info : public QObject
{
    Q_OBJECT
public:
    connected_object_info();
    ~connected_object_info();
    QObject* plugin;
    QString key;
    QMetaMethod method;
    struct connection_object
    {
        QMetaMethod meta_method;
        QMetaObject::Connection connection;
    };

    QVector<connection_object> connections;

    void addConnection(const QMetaMethod& method, const QMetaObject::Connection& connection, QObject* plugin);
signals:
    void onConnect(int code, QString object_name, QString connection_key);
    void onDisconnect(int code, QString object_name, QString connection_key);
};
