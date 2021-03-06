#include "notificationmanager.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include <QIcon>
#include <QtDebug>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "core.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::DBusManager;

NotificationManager::NotificationManager (QObject *parent)
: QObject (parent)
{
	if (!QDBusConnection::sessionBus ().interface ()->
			isServiceRegistered ("org.kde.VisualNotifications"))
	{
		qWarning () << Q_FUNC_INFO
			<< QDBusConnection::sessionBus ().interface ()->registeredServiceNames ().value ();
		return;
	}

	Connection_.reset (new QDBusInterface ("org.kde.VisualNotifications",
				"/VisualNotifications"));
	if (!Connection_->isValid ())
	{
		qWarning () << Q_FUNC_INFO
			<< Connection_->lastError ();
	}
}

void NotificationManager::HandleFinishedNotification (IHookProxy *proxy,
		const QString& msg, bool show)
{
	if (!Connection_.get () || !show)
		return;

	QByteArray iconData;
	{
		QDataStream arg (&iconData, QIODevice::WriteOnly);
		arg << qApp->windowIcon ().pixmap (64, 64);
	}

	QList<QVariant> arguments;
	arguments << "LeechCraft"
		<< uint (0)
		<< QString ()
		<< QString ("LeechCraft")
		<< QString ()
		<< msg
		<< QStringList ()
		<< QVariantMap ()
		<< Core::Instance ().GetProxy ()->GetSettingsManager ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000;

	if (Connection_->callWithArgumentList (QDBus::BlockWithGui,
			"Notify", arguments).type () == QDBusMessage::ErrorMessage)
		qWarning () << Q_FUNC_INFO
			<< Connection_->lastError ();
	else
		proxy->CancelDefault ();
}

