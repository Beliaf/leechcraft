#ifndef PLUGINS_AGGREGATOR_SINGLEREGEXP_H
#define PLUGINS_AGGREGATOR_SINGLEREGEXP_H
#include <QDialog>
#include "ui_singleregexp.h"

class QString;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class SingleRegexp : public QDialog
			{
				Q_OBJECT

				Ui::SingleRegexp Ui_;
			public:
				SingleRegexp (const QString& = QString (), const QString& = QString (), bool = false, QWidget* = 0);
				QString GetTitle () const;
				QString GetBody () const;
			private slots:
				void lineEdited (const QString&, QWidget* = 0);
			};
		};
	};
};

#endif

