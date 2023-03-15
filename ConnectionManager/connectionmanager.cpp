#include "connectionmanager.h"

#include <QtCore/QDebug>

ConnectionManager::ConnectionManager()
{

}

ConnectionManager::~ConnectionManager()
{

}

ConnectionManager::AddConnectionResult ConnectionManager::addEvent(QObject* plugin, const QString& key, const char* signal_name, options input)
{
	++signal_name; //skip code

	int signal_index = plugin->metaObject()->indexOfSignal(signal_name);
	if (signal_index == -1)
		return AddConnectionResult::SignalNotExist;

	return addEventByIndex(plugin, key, signal_index, input);
}

ConnectionManager::AddConnectionResult ConnectionManager::addEventByIndex(QObject* plugin, const QString& key, const int& signal_index, const options& input)
{
	const auto iter = std::find_if(events.begin(), events.end(), [&key](connected_object_info* const& elem)
		{
			return elem->key == key;
		});

	if (iter != events.end())
		return AddConnectionResult::EventAlreadyExist;
	else
	{
		connected_object_info* info = new connected_object_info;
		info->key = key;
		info->plugin = plugin;

		if (signal_index == -1)
			return AddConnectionResult::SignalNotExist;

		const auto plugin_meta_object = plugin->metaObject();

		info->method = plugin_meta_object->method(signal_index);

		const auto slot_names = getSlotNamesByInputParams(input);
		const QString on_connect_slot_name = slot_names.first;
		const QString on_disconnect_slot_name = slot_names.second;

		if (!on_connect_slot_name.isEmpty())
		{			
			QByteArray ba = on_connect_slot_name.toLocal8Bit();
			char* datachar = ba.data();
			int methodIndex = plugin_meta_object->indexOfMethod(datachar);
			if (methodIndex == -1)
				return AddConnectionResult::OnConnectSlotNotExist;

			const auto slotMethod = plugin_meta_object->method(methodIndex);

			int index = -1;

			auto func = (&connected_object_info::onConnect);
			void* args[] = { &index, reinterpret_cast<void**>(&func) };
			info->metaObject()->static_metacall(QMetaObject::IndexOfMethod, 0, args);

			const auto methodOffset = info->metaObject()->methodOffset();
			index += methodOffset;

			const auto signalMethod = info->metaObject()->method(index);

			if (!QMetaObject::checkConnectArgs(signalMethod, slotMethod))
				return AddConnectionResult::OnConnectSlotNotCompatible;

			const auto res = QObject::connect(info, signalMethod, plugin, slotMethod, Qt::UniqueConnection);
		}

		if (!on_disconnect_slot_name.isEmpty())
		{
			QByteArray ba = on_disconnect_slot_name.toLocal8Bit();
			char* datachar = ba.data();
			int methodIndex = plugin_meta_object->indexOfMethod(datachar);
			if (methodIndex == -1)
				return AddConnectionResult::OnDisconnectSlotNotExist;

			const auto slotMethod = plugin_meta_object->method(methodIndex);

			int index = -1;

			auto func = (&connected_object_info::onDisconnect);
			void* args[] = { &index, reinterpret_cast<void**>(&func) };
			info->metaObject()->static_metacall(QMetaObject::IndexOfMethod, 0, args);

			const auto methodOffset = info->metaObject()->methodOffset();
			index += methodOffset;

			const auto signalMethod = info->metaObject()->method(index);

			if (!QMetaObject::checkConnectArgs(signalMethod, slotMethod))
				return AddConnectionResult::OnDisconnectSlotNotCompatible;

			const auto res = QObject::connect(info, signalMethod, plugin, slotMethod, Qt::UniqueConnection);
		}

		QObject::connect(plugin, &QObject::destroyed, this, &ConnectionManager::onDestroyedConnectionObject);

		events.push_back(info);

		update_connections();
	}

	return AddConnectionResult::Successful;
}

ConnectionManager::AddConnectionResult ConnectionManager::addEventByPointer(QObject* plugin, const QString& key, void** function, const options& input)
{
	if (!function)
		return AddConnectionResult::NullFunctionPointer;

	auto plugin_meta_object = plugin->metaObject();
	if (!plugin_meta_object)
		return AddConnectionResult::PluginNotQtObject;

	int index = -1;
	void* args[] = { &index, function };
	plugin_meta_object->static_metacall(QMetaObject::IndexOfMethod, 0, args);

	if (index == -1)
		return AddConnectionResult::SignalNotExist;

	const auto methodOffset = plugin_meta_object->methodOffset();
	index += methodOffset;

	return addEventByIndex(plugin, key, index, input);
}

ConnectionManager::AddConnectionResult ConnectionManager::addSubscription(QObject* plugin, const QString& key, const char* slot_name, options input)
{
	const auto iter = std::find_if(subscriptions.begin(), subscriptions.end(), [&key, &plugin](connected_object_info* const& elem)
		{
			return ((elem->key == key) && (elem->plugin == plugin));
		});

	if (iter != subscriptions.end())
		return AddConnectionResult::EventAlreadyExist;
	else
	{
		connected_object_info* info = new connected_object_info;
		info->key = key;
		info->plugin = plugin;

		const auto plugin_meta_object = plugin->metaObject();

		{
			++slot_name;
			int methodIndex = plugin_meta_object->indexOfMethod(slot_name);
			if (methodIndex == -1)
				return AddConnectionResult::SlotNotExist;

			info->method = plugin->metaObject()->method(methodIndex);
		}

		const auto slot_names = getSlotNamesByInputParams(input);
		const auto on_connect_slot_name = slot_names.first;
		const auto on_disconnect_slot_name = slot_names.second;

		if (!on_connect_slot_name.isEmpty())
		{
			QByteArray ba = on_connect_slot_name.toLocal8Bit();
			char* datachar = ba.data();
			int methodIndex = plugin_meta_object->indexOfMethod(datachar);
			if (methodIndex == -1)
				return AddConnectionResult::OnConnectSlotNotExist;

			const auto slotMethod = plugin_meta_object->method(methodIndex);

			int index = -1;

			auto func = (&connected_object_info::onConnect);
			void* args[] = { &index, reinterpret_cast<void**>(&func) };
			info->metaObject()->static_metacall(QMetaObject::IndexOfMethod, 0, args);

			const auto methodOffset = info->metaObject()->methodOffset();
			index += methodOffset;

			const auto signalMethod = info->metaObject()->method(index);

			if (!QMetaObject::checkConnectArgs(signalMethod, slotMethod))
				return AddConnectionResult::OnConnectSlotNotCompatible;

			const auto res = QObject::connect(info, signalMethod, plugin, slotMethod, Qt::UniqueConnection);
		}

		if (!on_disconnect_slot_name.isEmpty())
		{
			QByteArray ba = on_disconnect_slot_name.toLocal8Bit();
			char* datachar = ba.data();
			int methodIndex = plugin_meta_object->indexOfMethod(datachar);
			if (methodIndex == -1)
				return AddConnectionResult::OnDisconnectSlotNotExist;

			const auto slotMethod = plugin_meta_object->method(methodIndex);

			int index = -1;

			auto func = (&connected_object_info::onDisconnect);
			void* args[] = { &index, reinterpret_cast<void**>(&func) };
			info->metaObject()->static_metacall(QMetaObject::IndexOfMethod, 0, args);

			const auto methodOffset = info->metaObject()->methodOffset();
			index += methodOffset;

			const auto signalMethod = info->metaObject()->method(index);

			if (!QMetaObject::checkConnectArgs(signalMethod, slotMethod))
				return AddConnectionResult::OnDisconnectSlotNotCompatible;

			const auto res = QObject::connect(info, signalMethod, plugin, slotMethod, Qt::UniqueConnection);
		}

		QObject::connect(plugin, &QObject::destroyed, this, &ConnectionManager::onDestroyedConnectionObject);

		subscriptions.push_back(info);

		update_connections();
	}

	return AddConnectionResult::Successful;
}

bool ConnectionManager::addSubscriptionByIndex(QObject* plugin, const QString& key, const int& slot_index)
{
	const auto iter = std::find_if(subscriptions.begin(), subscriptions.end(), [&key, &plugin](connected_object_info* const& elem)
		{
			return ((elem->key == key) && (elem->plugin == plugin));
		});

	if (iter != subscriptions.end())
		return false;
	else
	{
		connected_object_info* info = new connected_object_info;
		info->key = key;
		info->plugin = plugin;

		if (slot_index == -1)
			return false;

		info->method = plugin->metaObject()->method(slot_index);

		QObject::connect(plugin, &QObject::destroyed, this, &ConnectionManager::onDestroyedConnectionObject);

		subscriptions.push_back(info);

		update_connections();
	}

	return true;
}

void ConnectionManager::removeEvent(QObject* plugin, const QString& key)
{
	removeEventObjects(plugin, key);
}

void ConnectionManager::removeEventObjects(QObject* plugin, const QString& key)
{
	const auto event_iter = std::find_if(events.begin(), events.end(), [&key, &plugin](connected_object_info* const& elem)
		{
			return ((elem->key == key) && (elem->plugin == plugin));
		});

	if (event_iter == events.end())
		return;

	const auto event_index = std::distance(events.begin(), event_iter);
	const auto event_obj = events.at(event_index);
	const auto event_method = event_obj->method;

	const auto event_obj_name = event_obj->plugin->objectName();

	for (int i = 0, isize = subscriptions.size(); i < isize; i++)
	{
		auto obj = subscriptions.at(i);
		if (obj->key != key)
			continue;

		const auto iter = std::find_if(obj->connections.begin(), obj->connections.end(), [&event_method](connected_object_info::connection_object const& elem)
			{
				return (elem.meta_method == event_method);
			});

		if (iter != obj->connections.end())
		{
			obj->connections.erase(iter);
			emit obj->onDisconnect(0, event_obj_name, key);
		}
	}

	events.remove(event_index);
	emit event_obj->onDisconnect(0, event_obj_name, key);
	event_obj->deleteLater();
}

void ConnectionManager::removeSubscription(QObject* plugin, const QString& key)
{
	removeSubscriptionObjects(plugin, key);
}

void ConnectionManager::removeSubscriptionObjects(QObject* plugin, const QString& key)
{
	const auto subscription_iter = std::find_if(subscriptions.begin(), subscriptions.end(), [&key, &plugin](connected_object_info* const& elem)
		{
			return ((elem->key == key) && (elem->plugin == plugin));
		});

	if (subscription_iter == subscriptions.end())
		return;

	const auto subscription_index = std::distance(subscriptions.begin(), subscription_iter);
	const auto subscription_obj = subscriptions.at(subscription_index);
	const auto subscription_method = subscription_obj->method;

	const auto subscription_obj_name = subscription_obj->plugin->objectName();

	for (int i = 0, isize = events.size(); i < isize; i++)
	{
		auto obj = events.at(i);
		if (obj->key != key)
			continue;

		const auto iter = std::find_if(obj->connections.begin(), obj->connections.end(), [&subscription_method](connected_object_info::connection_object const& elem)
			{
				return (elem.meta_method == subscription_method);
			});

		if (iter != obj->connections.end())
		{
			obj->connections.erase(iter);
			emit obj->onDisconnect(0, subscription_obj_name, key);
		}
	}

	subscriptions.remove(subscription_index);
	emit subscription_obj->onDisconnect(0, subscription_obj_name, key);
	subscription_obj->deleteLater();
}

bool ConnectionManager::update_connections()
{
	bool updated = false;
	for (int i = events.size() - 1; i >= 0; i--)
	{
		const auto sender = events.at(i);
		const auto sender_key = sender->key;
		const auto sender_object = sender->plugin;
		const auto& sender_method = sender->method;
		const auto sender_method_index = sender_method.methodIndex();

		{
			bool need_remove = false;

			if (!sender_object)
				need_remove = true;
			else if (!sender_method.isValid())
				need_remove = true;
			else if (sender_method.methodType() != QMetaMethod::Signal)
				need_remove = true;
			else if (sender_method_index == -1)
				need_remove = true;

			if (need_remove)
			{
				removeEventObjects(sender_object, sender_key);
				continue;
			}
		}

		for (int j = subscriptions.size() - 1; j >= 0; j--)
		{
			const auto receiver = subscriptions.at(j);
			const auto receiver_key = receiver->key;
			const auto receiver_object = receiver->plugin;
			const auto& receiver_method = receiver->method;
			const auto receiver_method_index = receiver_method.methodIndex();

			{
				bool need_remove = false;

				if (!receiver_object)
					need_remove = true;
				else if (!receiver_method.isValid())
					need_remove = true;
				else if (receiver_method.methodType() == QMetaMethod::Constructor)
					need_remove = true;
				else if (receiver_method_index == -1)
					need_remove = true;

				if (need_remove)
				{
					removeSubscriptionObjects(receiver_object, receiver_key);
					continue;
				}
			}

			if (sender_key != receiver_key)
				continue;

			bool can_connect = QMetaObject::checkConnectArgs(sender_method, receiver_method);
			if (can_connect)
			{
				const auto iter1 = std::find_if(sender->connections.begin(), sender->connections.end(), [&receiver_method](connected_object_info::connection_object const& elem)
					{
						return (elem.meta_method == receiver_method);
					});

				const auto iter2 = std::find_if(receiver->connections.begin(), receiver->connections.end(), [&sender_method](connected_object_info::connection_object const& elem)
					{
						return (elem.meta_method == sender_method);
					});

				// both not connected
				if ((iter1 == sender->connections.end()) && (iter2 == receiver->connections.end()))
				{
					bool res = makeConnection(sender, sender_method, receiver, receiver_method);
					if (res)
						updated = true;
				}
				// both connected
				else if ((iter1 != sender->connections.end()) && (iter2 != receiver->connections.end()))
				{
					continue;
				}
				// one connected, other not. remove excess and reconnect
				else
				{
					if (iter1 != sender->connections.end())
						sender->connections.erase(iter1);

					if (iter2 != receiver->connections.end())
						receiver->connections.erase(iter2);

					bool res = makeConnection(sender, sender_method, receiver, receiver_method);
					if (res)
						updated = true;
				}
			}
		}
	}

	return updated;
}

bool ConnectionManager::makeConnection(connected_object_info* sender, const QMetaMethod& signal, connected_object_info* receiver, const QMetaMethod& slot)
{
	const auto connection = QObject::connect(sender->plugin, signal, receiver->plugin, slot, Qt::UniqueConnection);

	if (connection)
	{
		sender->addConnection(slot, connection, receiver->plugin);
		receiver->addConnection(signal, connection, sender->plugin);
	}

	return connection;
}

connected_object_info::connected_object_info()
{
	int t = 0;
}

connected_object_info::~connected_object_info()
{
	for (const auto& connection : connections)
	{
		QObject::disconnect(connection.connection);
	}

	connections.clear();
}

void connected_object_info::addConnection(const QMetaMethod& method, const QMetaObject::Connection& connection, QObject* plugin)
{
	{
		connection_object obj;
		obj.meta_method = method;
		obj.connection = connection;
		connections.push_back(obj);
	}

	const auto object_name = plugin->objectName();

	emit onConnect(0, object_name, key);
}

QPair<QString, QString> ConnectionManager::getSlotNamesByInputParams(options input)
{
	if (input.on_connect_slot_name)
		++input.on_connect_slot_name; //skip code
	if (input.on_disconnect_slot_name)
		++input.on_disconnect_slot_name; //skip code

	QString on_connect_slot_name, on_disconnect_slot_name;
	if (input.on_connect_slot_name)
		on_connect_slot_name = QMetaObject::normalizedSignature(input.on_connect_slot_name);
	if (input.on_disconnect_slot_name)
		on_disconnect_slot_name = QMetaObject::normalizedSignature(input.on_disconnect_slot_name);

	return qMakePair(on_connect_slot_name, on_disconnect_slot_name);
}

void ConnectionManager::onDestroyedConnectionObject(QObject* obj)
{
	for (int i = events.size() - 1; i >= 0; i--)
	{
		const auto connected_object = events.at(i);
		if (connected_object->plugin == obj)
		{
			const auto key = connected_object->key;
			removeEventObjects(obj, key);
		}
	}

	for (int i = subscriptions.size() - 1; i >= 0; i--)
	{
		const auto connected_object = subscriptions.at(i);
		if (connected_object->plugin == obj)
		{
			const auto key = connected_object->key;
			removeSubscriptionObjects(obj, key);
		}
	}
}

