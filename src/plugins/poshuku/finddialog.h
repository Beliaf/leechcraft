#ifndef PLUGINS_POSHUKU_FINDDIALOG_H
#define PLUGINS_POSHUKU_FINDDIALOG_H
#include <QWebPage>
#include "notification.h"
#include "ui_finddialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FindDialog : public Notification
			{
				Q_OBJECT

				Ui::FindDialog Ui_;
			public:
				FindDialog (QWidget* = 0);
				virtual ~FindDialog ();

				void SetSuccessful (bool);
			private slots:
				void on_Pattern__textChanged (const QString&);
				void on_FindButton__released ();
				void reject ();
			signals:
				void next (const QString&, QWebPage::FindFlags);
			};
		};
	};
};

#endif

