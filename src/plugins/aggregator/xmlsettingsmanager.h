#ifndef PLUGINS_AGGREGATOR_XMLSETTINGSMANAGER_H
#define PLUGINS_AGGREGATOR_XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class XmlSettingsManager : public LeechCraft::Util::BaseSettingsManager
			{
				Q_OBJECT
			public:
				XmlSettingsManager ();
				static XmlSettingsManager* Instance ();
			protected:
				virtual QSettings* BeginSettings () const;
				virtual void EndSettings (QSettings*) const;
			};
		};
	};
};

#endif

