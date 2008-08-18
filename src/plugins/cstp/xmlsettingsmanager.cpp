#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

XmlSettingsManager::XmlSettingsManager ()
{
    BaseSettingsManager::Init ();
}

XmlSettingsManager& XmlSettingsManager::Instance ()
{
	static XmlSettingsManager xsm;
	return xsm;
}

QSettings* XmlSettingsManager::BeginSettings () const
{
	QSettings *settings = new QSettings (
			Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CSTP");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings* settings) const
{
}

