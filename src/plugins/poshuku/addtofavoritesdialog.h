#ifndef PLUGINS_POSHUKU_ADDTOFAVORITESDIALOG_H
#define PLUGINS_POSHUKU_ADDTOFAVORITESDIALOG_H
#include <memory>
#include <QDialog>
#include <plugininterface/tagscompleter.h>
#include "ui_addtofavoritesdialog.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsCompletionModel;
	};

	namespace Plugins
	{
		namespace Poshuku
		{
			class AddToFavoritesDialog : public QDialog
			{
				Q_OBJECT

				Ui::AddToFavoritesDialog Ui_;

				std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsCompleter_;
			public:
				AddToFavoritesDialog (const QString&,
						const QString&,
						QWidget* = 0);
				virtual ~AddToFavoritesDialog ();

				QString GetTitle () const;
				QStringList GetTags () const;
			};
		};
	};
};

#endif

