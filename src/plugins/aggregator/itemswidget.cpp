#include "itemswidget.h"
#include <memory>
#include <QFileInfo>
#include <QHeaderView>
#include <plugininterface/categoryselector.h>
#include <plugininterface/proxy.h>
#include "core.h"
#include "itemsfiltermodel.h"
#include "xmlsettingsmanager.h"

using LeechCraft::Util::CategorySelector;

struct ItemsWidget_Impl
{
	Ui::ItemsWidget Ui_;

	bool HideInfoPanel_;

    QAction *ActionMarkItemAsUnread_;
    QAction *ActionAddToItemBucket_;

	std::auto_ptr<ItemsFilterModel> ItemsFilterModel_;
	std::auto_ptr<CategorySelector> ItemCategorySelector_;
};

ItemsWidget::ItemsWidget (QWidget *parent)
: QWidget (parent)
{
	Impl_ = new ItemsWidget_Impl;
	Impl_->HideInfoPanel_ = false;
	Impl_->ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"),
			this);
	Impl_->ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");

	Impl_->ActionAddToItemBucket_ = new QAction (tr ("Add to item bucket"),
			this);
	Impl_->ActionAddToItemBucket_->setObjectName ("ActionAddToItemBucket_");

	Impl_->Ui_.setupUi (this);

	Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);
	Impl_->ItemsFilterModel_.reset (new ItemsFilterModel (this));
	Impl_->ItemsFilterModel_->setSourceModel (&Core::Instance ());
	connect (&Core::Instance (),
			SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
			Impl_->ItemsFilterModel_.get (),
			SLOT (invalidate ()));
	Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
	Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
	Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());

	Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsUnread_);
	Impl_->Ui_.Items_->addAction (Impl_->ActionAddToItemBucket_);
	Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
	connect (Impl_->Ui_.SearchLine_,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (updateItemsFilter ()));
	connect (Impl_->Ui_.SearchType_,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (updateItemsFilter ()));
	QHeaderView *itemsHeader = Impl_->Ui_.Items_->header ();
	QFontMetrics fm = fontMetrics ();
	int dateTimeSize = fm.width (QDateTime::currentDateTime ()
			.toString (Qt::SystemLocaleShortDate) + "__");
	itemsHeader->resizeSection (0,
			fm.width ("Average news article size is about this width or "
				"maybe bigger, because they are bigger"));
	itemsHeader->resizeSection (1,
			dateTimeSize);
	connect (Impl_->Ui_.Items_->header (),
			SIGNAL (sectionClicked (int)),
			this,
			SLOT (makeCurrentItemVisible ()));

	Impl_->ItemCategorySelector_.reset (new CategorySelector ());
	connect (Impl_->ItemCategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			Impl_->ItemsFilterModel_.get (),
			SLOT (categorySelectionChanged (const QStringList&)));

	connect (Impl_->Ui_.ItemLink_,
			SIGNAL (linkActivated (const QString&)),
			&Core::Instance (),
			SLOT (openLink (const QString&)));

	connect (Impl_->Ui_.Items_->selectionModel (),
			SIGNAL (selectionChanged (const QItemSelection&,
					const QItemSelection&)),
			this,
			SLOT (currentItemChanged (const QItemSelection&)));

	currentItemChanged (QItemSelection ());

	QList<QByteArray> viewerSettings;
	viewerSettings << "StandardFont"
		<< "FixedFont"
		<< "SerifFont"
		<< "SansSerifFont"
		<< "CursiveFont"
		<< "FantasyFont"
		<< "MinimumFontSize"
		<< "DefaultFontSize"
		<< "DefaultFixedFontSize"
		<< "AutoLoadImages"
		<< "AllowJavaScript";
	XmlSettingsManager::Instance ()->RegisterObject (viewerSettings,
			this, "viewerSettingsChanged");

	viewerSettingsChanged ();
}

ItemsWidget::~ItemsWidget ()
{
	disconnect (Impl_->ItemsFilterModel_.get (), 0, this, 0);
	disconnect (Impl_->ItemCategorySelector_.get (), 0, this, 0);
	delete Impl_;
}

void ItemsWidget::SetHtml (const QString& title,
		const QString& description,
		const QList<Enclosure>& enclosures)
{
	QString result = "<div style='background: lightgray; "
		"border: 1px solid #000000; "
		"padding-left: 2em; "
		"padding-right: 2em;'>";
	result += title;
	result += "</div>";
	result += description;
	for (QList<Enclosure>::const_iterator i = enclosures.begin (),
			end = enclosures.end (); i != end; ++i)
	{
		result += "<div style='background: lightgray; "
			"border: 1px solid #333333; "
			"padding-top: 1em; "
			"padding-bottom: 1em; "
			"padding-left: 2em; "
			"padding-right: 2em;'>";
		if (i->Length_ > 0)
			result += tr ("File of type %1, size %2:<br />")
				.arg (i->Type_)
				.arg (LeechCraft::Util::Proxy::Instance ()->
						MakePrettySize (i->Length_));
		else
			result += tr ("File of type %1 and unknown length:<br />")
				.arg (i->Type_);
		result += QString ("<a href='%1'>%2</a>")
			.arg (i->URL_)
			.arg (QFileInfo (QUrl (i->URL_).path ()).fileName ());
		if (!i->Lang_.isEmpty ())
			result += tr ("<br />Specified language: %1")
				.arg (i->Lang_);
		result += "</div>";
	}
	Impl_->Ui_.ItemView_->setHtml (result);
}

void ItemsWidget::SetLink (QString link)
{
	QString shortLink;
	Impl_->Ui_.ItemLink_->setToolTip (link);
	if (link.size () >= 40)
		shortLink = link.left (18) + "..." + link.right (18);
	else
		shortLink = link;
	if (QUrl (link).isValid ())
	{
		link.insert (0,"<a href=\"");
		link.append ("\">" + shortLink + "</a>");
		Impl_->Ui_.ItemLink_->setText (link);
	}
	else
		Impl_->Ui_.ItemLink_->setText (shortLink);
}

void ItemsWidget::SetCategory (const QStringList& categories)
{
	QString category = categories.join ("; ").left (60);
	if (category.isEmpty ())
	{
		Impl_->Ui_.ItemCategory_->hide ();
		Impl_->Ui_.ItemCategoryLabel_->hide ();
	}
	else
	{
		Impl_->Ui_.ItemCategory_->setText (category);
		Impl_->Ui_.ItemCategory_->show ();
		Impl_->Ui_.ItemCategoryLabel_->show ();
	}
}

void ItemsWidget::SetPubDate (const QDateTime& pubDate)
{
	if (pubDate.isValid ())
	{
		Impl_->Ui_.ItemPubDate_->setText (pubDate.toString ());
		Impl_->Ui_.ItemPubDate_->show ();
		Impl_->Ui_.ItemPubDateLabel_->show ();
	}
	else
	{
		Impl_->Ui_.ItemPubDate_->hide ();
		Impl_->Ui_.ItemPubDateLabel_->hide ();
	}
}

void ItemsWidget::SetCommentsLabel (int numComments)
{
	if (numComments >= 0)
	{
		Impl_->Ui_.ItemComments_->show ();
		Impl_->Ui_.ItemCommentsLabel_->show ();

		QString text = QString::number (numComments);
		Impl_->Ui_.ItemComments_->setText (text);
	}
	else
	{
		Impl_->Ui_.ItemComments_->hide ();
		Impl_->Ui_.ItemCommentsLabel_->hide ();
	}
}

void ItemsWidget::SetAuthor (const QString& itemAuthor)
{
	if (itemAuthor.isEmpty ())
	{
		Impl_->Ui_.ItemAuthor_->hide ();
		Impl_->Ui_.ItemAuthorLabel_->hide ();
	}
	else
	{
		Impl_->Ui_.ItemAuthor_->setText (itemAuthor);
		Impl_->Ui_.ItemAuthor_->show ();
		Impl_->Ui_.ItemAuthorLabel_->show ();
	}
}

void ItemsWidget::SetHideRead (bool hide)
{
	Impl_->ItemsFilterModel_->SetHideRead (hide);
}

void ItemsWidget::ChannelChanged (const QModelIndex& mapped)
{
	Impl_->Ui_.Items_->scrollToTop ();
	currentItemChanged (QItemSelection ());

	QStringList allCategories = Core::Instance ().GetCategories (mapped);
	if (allCategories.size ())
	{
		Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
		Impl_->ItemCategorySelector_->selectAll ();
		Impl_->Ui_.ItemCategoriesButton_->show ();
	}
	else
		Impl_->Ui_.ItemCategoriesButton_->hide ();

	Impl_->ItemsFilterModel_->categorySelectionChanged (allCategories);
}

void ItemsWidget::HideInfoPanel ()
{
	Impl_->HideInfoPanel_ = true;
}

void ItemsWidget::on_ActionMarkItemAsUnread__triggered ()
{
    QModelIndexList indexes = Impl_->Ui_.Items_->
		selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkItemAsUnread (Impl_->
				ItemsFilterModel_->mapToSource (indexes.at (i)));
}

void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
{
    Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
			Qt::CaseSensitive : Qt::CaseInsensitive);
}

void ItemsWidget::on_ActionAddToItemBucket__triggered ()
{
	Core::Instance ().AddToItemBucket (Impl_->ItemsFilterModel_->
			mapToSource (Impl_->Ui_.Items_->selectionModel ()->
				currentIndex ()));
}

void ItemsWidget::on_ItemCommentsSubscribe__released ()
{
    QModelIndex selected = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
	Core::Instance ().SubscribeToComments (Impl_->ItemsFilterModel_->
			mapToSource (selected));
}

void ItemsWidget::on_ItemCategoriesButton__released ()
{
	Impl_->ItemCategorySelector_->move (QCursor::pos ());
	Impl_->ItemCategorySelector_->show ();
}

void ItemsWidget::viewerSettingsChanged ()
{
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::StandardFont,
			XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FixedFont,
			XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SerifFont,
			XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SansSerifFont,
			XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::CursiveFont,
			XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FantasyFont,
			XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());

	Impl_->Ui_.ItemView_->settings ()->setFontSize (QWebSettings::MinimumFontSize,
			XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
	Impl_->Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
	Impl_->Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
	Impl_->Ui_.ItemView_->settings ()->setAttribute (QWebSettings::AutoLoadImages,
			XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
	Impl_->Ui_.ItemView_->settings ()->setAttribute (QWebSettings::JavascriptEnabled,
			XmlSettingsManager::Instance ()->property ("AllowJavaScript").toBool ());
}

void ItemsWidget::currentItemChanged (const QItemSelection& selection)
{
	QModelIndexList indexes = selection.indexes ();

	Impl_->Ui_.ItemInfo_->setVisible (!(Impl_->HideInfoPanel_ ||
			!selection.size ()));

	QModelIndex sindex;
	if (indexes.size ())
		sindex = Impl_->ItemsFilterModel_->mapToSource (indexes.at (0));

	if (!sindex.isValid () || indexes.size () != 2)
	{
		Impl_->Ui_.ItemView_->setHtml ("");
		Impl_->Ui_.ItemAuthor_->hide ();
		Impl_->Ui_.ItemAuthorLabel_->hide ();
		Impl_->Ui_.ItemCategory_->hide ();
		Impl_->Ui_.ItemCategoryLabel_->hide ();
		Impl_->Ui_.ItemLink_->setText ("");
		Impl_->Ui_.ItemLinkLabel_->hide ();
		Impl_->Ui_.ItemPubDate_->hide ();
		Impl_->Ui_.ItemPubDateLabel_->hide ();
		Impl_->Ui_.ItemComments_->hide ();
		Impl_->Ui_.ItemCommentsLabel_->hide ();
		Impl_->Ui_.ItemCommentsSubscribe_->hide ();
		return;
	}

	Core::Instance ().Selected (sindex);

	Item_ptr item = Core::Instance ().GetItem (sindex);

	SetHtml (item->Title_, item->Description_, item->Enclosures_);
	connect (Impl_->Ui_.ItemView_->page ()->networkAccessManager (),
			SIGNAL (sslErrors (QNetworkReply*, const QList<QSslError>&)),
			&Core::Instance (),
			SLOT (handleSslError (QNetworkReply*)));

	if (Impl_->HideInfoPanel_)
		return;

	SetAuthor (item->Author_);
	SetCategory (item->Categories_);
	SetLink (item->Link_);
	SetPubDate (item->PubDate_);
	SetCommentsLabel (item->NumComments_);

	QString commentsRSS = item->CommentsLink_;
	Impl_->Ui_.ItemCommentsSubscribe_->setVisible (!commentsRSS.isEmpty ());
}

void ItemsWidget::makeCurrentItemVisible ()
{
	QModelIndex item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
	if (item.isValid ())
		Impl_->Ui_.Items_->scrollTo (item);
}

void ItemsWidget::updateItemsFilter ()
{
	int section = Impl_->Ui_.SearchType_->currentIndex ();
	QString text = Impl_->Ui_.SearchLine_->text ();
	switch (section)
	{
	case 1:
		Impl_->ItemsFilterModel_->setFilterWildcard (text);
		break;
	case 2:
		Impl_->ItemsFilterModel_->setFilterRegExp (text);
		break;
	default:
		Impl_->ItemsFilterModel_->setFilterFixedString (text);
		break;
	}
}
