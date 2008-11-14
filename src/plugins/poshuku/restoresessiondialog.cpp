#include "restoresessiondialog.h"

RestoreSessionDialog::RestoreSessionDialog (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
}

RestoreSessionDialog::~RestoreSessionDialog ()
{
}

void RestoreSessionDialog::AddPair (const QString& title,
		const QString& url)
{
	QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Pages_,
			QStringList (title) << url);
	item->setData (0, Qt::CheckStateRole, Qt::Checked);
}

QStringList RestoreSessionDialog::GetSelectedURLs () const
{
	QStringList result;
	for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
			i < end; ++i)
		if (Ui_.Pages_->topLevelItem (i)->
				data (0, Qt::CheckStateRole).toInt () == Qt::Checked)
			result << Ui_.Pages_->topLevelItem (i)->
				data (1, Qt::DisplayRole).toString ();
	return result;
}

void RestoreSessionDialog::on_SelectAll__released ()
{
	for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
			i < end; ++i)
		Ui_.Pages_->topLevelItem (i)->setData (0, Qt::CheckStateRole,
				Qt::Checked);
}

void RestoreSessionDialog::on_SelectNone__released ()
{
	for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
			i < end; ++i)
		Ui_.Pages_->topLevelItem (i)->setData (0, Qt::CheckStateRole,
				Qt::Unchecked);
}
