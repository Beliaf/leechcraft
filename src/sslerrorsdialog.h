#ifndef SSLERRORSDIALOG_H
#define SSLERRORSDIALOG_H
#include <QDialog>
#include <QList>
#include <QSslError>
#include "ui_sslerrorsdialog.h"

namespace LeechCraft
{
	class SslErrorsDialog : public QDialog
	{
		Q_OBJECT

		Ui::SslErrorsDialog Ui_;
	public:
		enum RememberChoice
		{
			RCNot
			, RCFile
			, RCHost
		};

		SslErrorsDialog (const QString&, const QList<QSslError>&, QWidget* = 0);
		virtual ~SslErrorsDialog ();

		RememberChoice GetRememberChoice () const;
	private:
		void PopulateTree (const QSslError&);
	};
};

#endif

