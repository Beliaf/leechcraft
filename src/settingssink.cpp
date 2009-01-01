#include "settingssink.h"
#include <interfaces/interfaces.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <QtDebug>

using namespace LeechCraft;

SettingsSink::SettingsSink (const QString& name,
		Util::XmlSettingsDialog* dialog,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	// Because Qt Designer inserts one.
	Ui_.Dialogs_->removeWidget (Ui_.Dialogs_->currentWidget ());

	Add (name, windowIcon (), dialog);
}

SettingsSink::~SettingsSink ()
{
}

void SettingsSink::AddDialog (const QObject *object)
{
	IInfo *info = qobject_cast<IInfo*> (object);
	IHaveSettings *ihs = qobject_cast<IHaveSettings*> (object);

	Add (info->GetName (), info->GetIcon (), ihs->GetSettingsDialog ());

	Ui_.Combobox_->setCurrentIndex (0);
}

void SettingsSink::Add (const QString& name, const QIcon& wicon,
		QWidget *widget)
{
	Ui_.Combobox_->addItem (wicon, name);
	Ui_.Dialogs_->addWidget (widget);
	adjustSize ();

	connect (this,
			SIGNAL (accepted ()),
			widget,
			SLOT (accept ()));
	connect (this,
			SIGNAL (rejected ()),
			widget,
			SLOT (reject ()));
}
