#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QQueue>
#include <QTimer>
#include <QTranslator>
#include <QCursor>
#include <QKeyEvent>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/util.h>
#include <plugininterface/proxy.h>
#include <plugininterface/categoryselector.h>
#include <plugininterface/tagscompleter.h>
#include <plugininterface/backendselector.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_mainwidget.h"
#include "itemsfiltermodel.h"
#include "channelsfiltermodel.h"
#include "aggregator.h"
#include "core.h"
#include "addfeed.h"
#include "itemsfiltermodel.h"
#include "channelsfiltermodel.h"
#include "xmlsettingsmanager.h"
#include "itembucket.h"
#include "regexpmatcherui.h"
#include "regexpmatchermanager.h"
#include "importopml.h"
#include "export.h"
#include "itembucket.h"
#include "importbinary.h"
#include "feedsettings.h"
#include "jobholderrepresentation.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			using LeechCraft::Util::TagsCompleter;
			using LeechCraft::Util::CategorySelector;
			using LeechCraft::ActionInfo;
			
			struct Aggregator_Impl
			{
				Ui::MainWidget Ui_;

				ItemsWidget *AdditionalInfo_;

				QToolBar *ToolBar_;
				QToolBar *ControlToolBar_;
				QAction *ActionAddFeed_;
				QAction *ActionUpdateFeeds_;
				QAction *ActionRemoveFeed_;
				QAction *ActionMarkChannelAsRead_;
				QAction *ActionMarkChannelAsUnread_;
				QAction *ActionChannelSettings_;
				QAction *ActionUpdateSelectedFeed_;
				QAction *ActionItemBucket_;
				QAction *ActionRegexpMatcher_;
				QAction *ActionHideReadItems_;
				QAction *ActionImportOPML_;
				QAction *ActionExportOPML_;
				QAction *ActionImportBinary_;
				QAction *ActionExportBinary_;

				QQueue<QString> ErrorQueue_;

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
				std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsLineCompleter_;
				std::auto_ptr<QSystemTrayIcon> TrayIcon_;
				std::auto_ptr<QTranslator> Translator_;
				std::auto_ptr<ItemBucket> ItemBucket_;
				std::auto_ptr<RegexpMatcherUi> RegexpMatcherUi_;

				QModelIndex SelectedRepr_;

				enum
				{
					EAActionAddFeed_,
					EAActionUpdateFeeds_,
					EAActionRemoveFeed_,
					EAActionMarkChannelAsRead_,
					EAActionMarkChannelAsUnread_,
					EAActionChannelSettings_,
					EAActionUpdateSelectedFeed_,
					EAActionItemBucket_,
					EAActionRegexpMatcher_,
					EAActionHideReadItems_,
					EAActionImportOPML_,
					EAActionExportOPML_,
					EAActionImportBinary_,
					EAActionExportBinary_
				};
			};
			
			Aggregator::~Aggregator ()
			{
			}
			
			void Aggregator::Init (ICoreProxy_ptr proxy)
			{
				Impl_ = new Aggregator_Impl;
				Impl_->Translator_.reset (LeechCraft::Util::InstallTranslator ("aggregator"));
			
				SetupActions ();
			
				Impl_->ToolBar_ = SetupMenuBar ();
				Impl_->ControlToolBar_ = SetupMenuBar ();
				Impl_->TrayIcon_.reset (new QSystemTrayIcon (QIcon (":/resources/images/aggregator.png"), this));
				Impl_->TrayIcon_->hide ();
				connect (Impl_->TrayIcon_.get (),
						SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
						this,
						SLOT (trayIconActivated ()));
			
				Core::Instance ().SetProxy (proxy);
			
				connect (&Core::Instance (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (showError (const QString&)));
				connect (&Core::Instance (),
						SIGNAL (showDownloadMessage (const QString&)),
						this,
						SIGNAL (downloadFinished (const QString&)));
				connect (&Core::Instance (),
						SIGNAL (unreadNumberChanged (int)),
						this,
						SLOT (unreadNumberChanged (int)));
				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**)));
			
				Impl_->XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				Impl_->XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");
				Impl_->XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
						new LeechCraft::Util::BackendSelector (XmlSettingsManager::Instance ()));
			
				Core::Instance ().DoDelayedInit ();
				Core::Instance ().GetJobHolderRepresentation ()->setParent (this);
			
				Impl_->AdditionalInfo_ = new ItemsWidget ();
				Impl_->AdditionalInfo_->HideInfoPanel ();
				Impl_->Ui_.setupUi (this);
			
				Impl_->Ui_.MergeItems_->setChecked (XmlSettingsManager::Instance ()->
						Property ("MergeItems", false).toBool ());
				Impl_->Ui_.ShowAsTape_->setChecked (XmlSettingsManager::Instance ()->
						Property ("TapeMode", false).toBool ());
			
				Impl_->RegexpMatcherUi_.reset (new RegexpMatcherUi (this));
			
				Impl_->ItemBucket_.reset (new ItemBucket (this));
			
				Impl_->Ui_.Feeds_->setModel (Core::Instance ().GetChannelsModel ());
				Impl_->Ui_.Feeds_->addAction (Impl_->ActionMarkChannelAsRead_);
				Impl_->Ui_.Feeds_->addAction (Impl_->ActionMarkChannelAsUnread_);
				QAction *sep = new QAction (Impl_->Ui_.Feeds_);
				sep->setSeparator (true);
				Impl_->Ui_.Feeds_->addAction (sep);
				Impl_->Ui_.Feeds_->addAction (Impl_->ActionChannelSettings_);
				Impl_->Ui_.Feeds_->setContextMenuPolicy (Qt::ActionsContextMenu);
				QHeaderView *channelsHeader = Impl_->Ui_.Feeds_->header ();
			
				QFontMetrics fm = fontMetrics ();
				int dateTimeSize = fm.width (QDateTime::currentDateTime ()
						.toString (Qt::SystemLocaleShortDate) + "__");
				channelsHeader->resizeSection (0, fm.width ("Average channel name"));
				channelsHeader->resizeSection (1, fm.width ("_9999_"));
				channelsHeader->resizeSection (2, dateTimeSize);
				connect (Impl_->Ui_.TagsLine_,
						SIGNAL (textChanged (const QString&)),
						Core::Instance ().GetChannelsModel (),
						SLOT (setFilterFixedString (const QString&)));
				connect (Impl_->Ui_.TagsLine_,
						SIGNAL (textChanged (const QString&)),
						&Core::Instance (),
						SLOT (tagsUpdated ()));
				connect (Impl_->Ui_.Feeds_->selectionModel (),
						SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
						this,
						SLOT (currentChannelChanged ()));
				connect (Impl_->ActionUpdateFeeds_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (updateFeeds ()));
			
				Impl_->TagsLineCompleter_.reset (new TagsCompleter (Impl_->Ui_.TagsLine_));
				Impl_->Ui_.TagsLine_->AddSelector ();
			
				Impl_->Ui_.MainSplitter_->setStretchFactor (0, 5);
				Impl_->Ui_.MainSplitter_->setStretchFactor (1, 9);
			
				connect (&RegexpMatcherManager::Instance (),
						SIGNAL (gotLink (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
			
				Core::Instance ().SetWidgets (Impl_->ControlToolBar_, Impl_->AdditionalInfo_);
				currentChannelChanged ();
			}
			
			void Aggregator::Release ()
			{
				disconnect (&Core::Instance (), 0, this, 0);
				disconnect (Core::Instance ().GetChannelsModel (), 0, this, 0);
				disconnect (Impl_->TagsLineCompleter_.get (), 0, this, 0);
				Impl_->TrayIcon_->hide ();
				delete Impl_;
				Core::Instance ().Release ();
			}
			
			QString Aggregator::GetName () const
			{
				return "Aggregator";
			}
			
			QString Aggregator::GetInfo () const
			{
				return tr ("RSS/Atom feed reader.");
			}
			
			QStringList Aggregator::Provides () const
			{
				return QStringList ("rss");
			}
			
			QStringList Aggregator::Needs () const
			{
				return QStringList ("http");
			}
			
			QStringList Aggregator::Uses () const
			{
				return QStringList ("webbrowser");
			}
			
			void Aggregator::SetProvider (QObject* object, const QString& feature)
			{
				Core::Instance ().SetProvider (object, feature);
			}
			
			QIcon Aggregator::GetIcon () const
			{
				return windowIcon ();
			}
			
			QWidget* Aggregator::GetTabContents ()
			{
				return this;
			}
			
			QToolBar* Aggregator::GetToolBar () const
			{
				return Impl_->ToolBar_;
			}
			
			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Aggregator::GetSettingsDialog () const
			{
				return Impl_->XmlSettingsDialog_;
			}
			
			QAbstractItemModel* Aggregator::GetRepresentation () const
			{
				return Core::Instance ().GetJobHolderRepresentation ();
			}
			
			void Aggregator::ItemSelected (const QModelIndex& index)
			{
				Impl_->SelectedRepr_ = index;
				Core::Instance ().CurrentChannelChanged (index, true);
			}
			
			bool Aggregator::CouldHandle (const LeechCraft::DownloadEntity& e) const
			{
				return Core::Instance ().CouldHandle (e);
			}
			
			void Aggregator::Handle (LeechCraft::DownloadEntity e)
			{
				AddFeed af (QString (e.Location_));
				if (af.exec () == QDialog::Accepted)
					Core::Instance ().AddFeed (e.Location_,
							af.GetTags ());
			}
			
#define _LC_MERGE(a) EA##a
			
#define _LC_SINGLE(a) \
				case Aggregator_Impl::_LC_MERGE(a): \
					Impl_->a->setShortcut (shortcut); \
					break;
			
#define _LC_TRAVERSER(z,i,array) \
				_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))
			
#define _LC_EXPANDER(Names) \
				switch (name) \
				{ \
					BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names) \
				}
			void Aggregator::SetShortcut (int name,
					const QKeySequence& shortcut)
			{
				_LC_EXPANDER ((ActionAddFeed_)
						(ActionUpdateFeeds_)
						(ActionRemoveFeed_)
						(ActionMarkChannelAsRead_)
						(ActionMarkChannelAsUnread_)
						(ActionChannelSettings_)
						(ActionUpdateSelectedFeed_)
						(ActionItemBucket_)
						(ActionRegexpMatcher_)
						(ActionHideReadItems_)
						(ActionImportOPML_)
						(ActionExportOPML_)
						(ActionImportBinary_)
						(ActionExportBinary_));
			}
			
#define _L(a) result [Aggregator_Impl::EA##a] = ActionInfo (Impl_->a->text (), \
					Impl_->a->shortcut (), Impl_->a->icon ())
			QMap<int, ActionInfo> Aggregator::GetActionInfo () const
			{
				QMap<int, ActionInfo> result;
				_L (ActionAddFeed_);
				_L (ActionUpdateFeeds_);
				_L (ActionRemoveFeed_);
				_L (ActionMarkChannelAsRead_);
				_L (ActionMarkChannelAsUnread_);
				_L (ActionChannelSettings_);
				_L (ActionUpdateSelectedFeed_);
				_L (ActionItemBucket_);
				_L (ActionRegexpMatcher_);
				_L (ActionHideReadItems_);
				_L (ActionImportOPML_);
				_L (ActionExportOPML_);
				_L (ActionImportBinary_);
				_L (ActionExportBinary_);
				return result;
			}
			
			void Aggregator::keyPressEvent (QKeyEvent *e)
			{
				if (e->modifiers () & Qt::ControlModifier)
				{
					QItemSelectionModel *channelSM = Impl_->Ui_.Feeds_->selectionModel ();
					QModelIndex currentChannel = channelSM->currentIndex ();
					int numChannels = Impl_->Ui_.Feeds_->model ()->rowCount ();
			
					QItemSelectionModel::SelectionFlags chanSF =
						QItemSelectionModel::Select |
						QItemSelectionModel::Clear |
						QItemSelectionModel::Rows;
					if (e->key () == Qt::Key_Less &&
							currentChannel.isValid () &&
							numChannels)
					{
						if (currentChannel.row () > 0)
						{
							QModelIndex next = currentChannel
								.sibling (currentChannel.row () - 1,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						else
						{
							QModelIndex next = currentChannel.sibling (numChannels - 1,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						return;
					}
					else if (e->key () == Qt::Key_Greater &&
							currentChannel.isValid () &&
							numChannels)
					{
						if (currentChannel.row () < numChannels - 1)
						{
							QModelIndex next = currentChannel
								.sibling (currentChannel.row () + 1,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						else
						{
							QModelIndex next = currentChannel.sibling (0,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						return;
					}
					else if ((e->key () == Qt::Key_Greater ||
							e->key () == Qt::Key_Less) &&
							numChannels &&
							!currentChannel.isValid ())
					{
						QModelIndex next = Impl_->Ui_.Feeds_->model ()->index (0, 0);
						channelSM->select (next, chanSF);
						channelSM->setCurrentIndex (next, chanSF);
					}
				}
				e->ignore ();
			}
			
			QToolBar* Aggregator::SetupMenuBar ()
			{
				QToolBar *bar = new QToolBar ();
			
				bar->addAction (Impl_->ActionAddFeed_);
				bar->addAction (Impl_->ActionRemoveFeed_);
				bar->addAction (Impl_->ActionUpdateSelectedFeed_);
				bar->addAction (Impl_->ActionUpdateFeeds_);
				bar->addSeparator ();
				bar->addAction (Impl_->ActionItemBucket_);
				bar->addAction (Impl_->ActionRegexpMatcher_);
				bar->addSeparator ();
				bar->addAction (Impl_->ActionImportOPML_);
				bar->addAction (Impl_->ActionExportOPML_);
				bar->addAction (Impl_->ActionImportBinary_);
				bar->addAction (Impl_->ActionExportBinary_);
				bar->addSeparator ();
				bar->addAction (Impl_->ActionHideReadItems_);
			
				return bar;
			}
			
			void Aggregator::SetupActions ()
			{
				Impl_->ActionAddFeed_ = new QAction (tr ("Add feed..."),
						this);
				Impl_->ActionAddFeed_->setObjectName ("ActionAddFeed_");
				Impl_->ActionAddFeed_->setProperty ("ActionIcon", "aggregator_add");
			
				Impl_->ActionUpdateFeeds_ = new QAction (tr ("Update all feeds"),
						this);
				Impl_->ActionUpdateFeeds_->setProperty ("ActionIcon", "aggregator_updateallfeeds");
			
				Impl_->ActionRemoveFeed_ = new QAction (tr ("Remove feed"),
						this);
				Impl_->ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");
				Impl_->ActionRemoveFeed_->setProperty ("ActionIcon", "aggregator_remove");
			
				Impl_->ActionMarkChannelAsRead_ = new QAction (tr ("Mark channel as read"),
						this);
				Impl_->ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");
			
				Impl_->ActionMarkChannelAsUnread_ = new QAction (tr ("Mark channel as unread"),
						this);
				Impl_->ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");
			
				Impl_->ActionChannelSettings_ = new QAction (tr ("Settings..."),
						this);
				Impl_->ActionChannelSettings_->setObjectName ("ActionChannelSettings_");
			
				Impl_->ActionUpdateSelectedFeed_ = new QAction (tr ("Update selected feed"),
						this);
				Impl_->ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");
				Impl_->ActionUpdateSelectedFeed_->setProperty ("ActionIcon", "aggregator_updateselectedfeed");
			
				Impl_->ActionItemBucket_ = new QAction (tr ("Item bucket..."),
						this);
				Impl_->ActionItemBucket_->setObjectName ("ActionItemBucket_");
				Impl_->ActionItemBucket_->setProperty ("ActionIcon", "aggregator_favorites");
			
				Impl_->ActionRegexpMatcher_ = new QAction (tr ("Regexp matcher..."),
						this);
				Impl_->ActionRegexpMatcher_->setObjectName ("ActionRegexpMatcher_");
				Impl_->ActionRegexpMatcher_->setProperty ("ActionIcon", "aggregator_filter");
			
				Impl_->ActionHideReadItems_ = new QAction (tr ("Hide read items"),
						this);
				Impl_->ActionHideReadItems_->setObjectName ("ActionHideReadItems_");
				Impl_->ActionHideReadItems_->setCheckable (true);
				Impl_->ActionHideReadItems_->setProperty ("ActionIcon", "aggregator_rssshow");
				Impl_->ActionHideReadItems_->setProperty ("ActionIconOff", "aggregator_rsshide");
			
				Impl_->ActionImportOPML_ = new QAction (tr ("Import from OPML..."),
						this);
				Impl_->ActionImportOPML_->setObjectName ("ActionImportOPML_");
				Impl_->ActionImportOPML_->setProperty ("ActionIcon", "aggregator_importopml");
			
				Impl_->ActionExportOPML_ = new QAction (tr ("Export to OPML..."),
						this);
				Impl_->ActionExportOPML_->setObjectName ("ActionExportOPML_");
				Impl_->ActionExportOPML_->setProperty ("ActionIcon", "aggregator_exportopml");
			
				Impl_->ActionImportBinary_ = new QAction (tr ("Import from binary..."),
						this);
				Impl_->ActionImportBinary_->setObjectName ("ActionImportBinary_");
				Impl_->ActionImportBinary_->setProperty ("ActionIcon", "aggregator_importbinary");
			
				Impl_->ActionExportBinary_ = new QAction (tr ("Export to binary..."),
						this);
				Impl_->ActionExportBinary_->setObjectName ("ActionExportBinary_");
				Impl_->ActionExportBinary_->setProperty ("ActionIcon", "aggregator_exportbinary");
			}
			
			void Aggregator::ScheduleShowError ()
			{
				if (Impl_->ErrorQueue_.size () > 1)
					return;
			
				QTimer::singleShot (500,
						this,
						SLOT (showError ()));
			}
			
			bool Aggregator::IsRepr ()
			{
				return Impl_->ControlToolBar_->isVisible ();
			}
			
			void Aggregator::showError (const QString& msg)
			{
				Impl_->ErrorQueue_.enqueue (msg);
				ScheduleShowError ();
			}
			
			void Aggregator::showError ()
			{
				while (Impl_->ErrorQueue_.size ())
				{
					QMessageBox::critical (this,
							tr ("Error"),
							Impl_->ErrorQueue_.dequeue ());
					QApplication::processEvents ();
				}
			}
			
			void Aggregator::on_ActionAddFeed__triggered ()
			{
				AddFeed af;
				if (af.exec () == QDialog::Accepted)
					Core::Instance ().AddFeed (af.GetURL (), af.GetTags ());
			}
			
			void Aggregator::on_ActionRemoveFeed__triggered ()
			{
				QModelIndex ds;
				if (IsRepr ())
					ds = Impl_->SelectedRepr_;
				else
					ds = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
			
				QString name = ds.sibling (ds.row (), 0).data ().toString ();
			
				QMessageBox mb (QMessageBox::Warning,
						tr ("Warning"),
						tr ("You are going to permanently remove the feed:"
							"<br />%1<br /><br />"
							"Are you are really sure that you want to do this?",
							"Feed removing confirmation").arg (name),
						QMessageBox::Ok | QMessageBox::Cancel,
						this);
				mb.setWindowModality (Qt::WindowModal);
				if (mb.exec () == QMessageBox::Ok)
					Core::Instance ().RemoveFeed (ds, IsRepr ());
			}
			
			void Aggregator::on_ActionMarkChannelAsRead__triggered ()
			{
				QModelIndexList indexes = Impl_->Ui_.Feeds_->selectionModel ()->selectedRows ();
				for (int i = 0; i < indexes.size (); ++i)
					Core::Instance ().MarkChannelAsRead (indexes.at (i));
			}
			
			void Aggregator::on_ActionMarkChannelAsUnread__triggered ()
			{
				QModelIndexList indexes = Impl_->Ui_.Feeds_->selectionModel ()->selectedRows ();
				for (int i = 0; i < indexes.size (); ++i)
					Core::Instance ().MarkChannelAsUnread (indexes.at (i));
			}
			
			void Aggregator::on_ActionChannelSettings__triggered ()
			{
				QModelIndex index = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
				if (!index.isValid ())
					return;
			
				index = Core::Instance ().GetChannelsModel ()->mapToSource (index);
			
				std::auto_ptr<FeedSettings> dia (new FeedSettings (index, this));
				dia->exec ();
			}
			
			void Aggregator::on_ActionUpdateSelectedFeed__triggered ()
			{
				bool isRepr = IsRepr ();
				QModelIndex current;
				if (isRepr)
					current = Impl_->SelectedRepr_;
				else
					current = Impl_->Ui_.Feeds_->
						selectionModel ()->currentIndex ();
			
				if (!current.isValid ())
				{
					qWarning () << Q_FUNC_INFO
						<< current
						<< isRepr;
					return;
				}
				Core::Instance ().UpdateFeed (current, isRepr);
			}
			
			void Aggregator::on_ActionItemBucket__triggered ()
			{
				Impl_->ItemBucket_->show ();
			}
			
			void Aggregator::on_ActionRegexpMatcher__triggered ()
			{
				Impl_->RegexpMatcherUi_->show ();
			}
			
			void Aggregator::on_ActionHideReadItems__triggered ()
			{
				bool hide = Impl_->ActionHideReadItems_->isChecked ();
				Impl_->Ui_.ItemsWidget_->SetHideRead (hide);
				Impl_->AdditionalInfo_->SetHideRead (hide);
			}
			
			void Aggregator::on_ActionImportOPML__triggered ()
			{
				ImportOPML importDialog;
				if (importDialog.exec () == QDialog::Rejected)
					return;
			
				Core::Instance ().AddFromOPML (importDialog.GetFilename (),
						importDialog.GetTags (),
						importDialog.GetMask ());
			}
			
			void Aggregator::on_ActionExportOPML__triggered ()
			{
				Export exportDialog (tr ("Export to OPML"),
						tr ("Select save file"),
						tr ("OPML files (*.opml);;"
							"XML files (*.xml);;"
							"All files (*.*)"), this);
				channels_shorts_t channels;
				Core::Instance ().GetChannels (channels);
				exportDialog.SetFeeds (channels);
				if (exportDialog.exec () == QDialog::Rejected)
					return;
			
				Core::Instance ().ExportToOPML (exportDialog.GetDestination (),
						exportDialog.GetTitle (),
						exportDialog.GetOwner (),
						exportDialog.GetOwnerEmail (),
						exportDialog.GetSelectedFeeds ());
			}
			
			void Aggregator::on_ActionImportBinary__triggered ()
			{
				ImportBinary import (this);
				if (import.exec () == QDialog::Rejected)
					return;
			
				Core::Instance ().AddFeeds (import.GetSelectedFeeds (),
						import.GetTags ());
			}
			
			void Aggregator::on_ActionExportBinary__triggered ()
			{
				Export exportDialog (tr ("Export to binary file"),
						tr ("Select save file"),
						tr ("Aggregator exchange files (*.lcae);;"
							"All files (*.*)"), this);
				channels_shorts_t channels;
				Core::Instance ().GetChannels (channels);
				exportDialog.SetFeeds (channels);
				if (exportDialog.exec () == QDialog::Rejected)
					return;
			
				Core::Instance ().ExportToBinary (exportDialog.GetDestination (),
						exportDialog.GetTitle (),
						exportDialog.GetOwner (),
						exportDialog.GetOwnerEmail (),
						exportDialog.GetSelectedFeeds ());
			}
			
			void Aggregator::on_MergeItems__toggled (bool merge)
			{
				Core::Instance ().SetMerge (merge);
				XmlSettingsManager::Instance ()->setProperty ("MergeItems", merge);
			}
			
			void Aggregator::on_ShowAsTape__toggled (bool tape)
			{
				Impl_->Ui_.ItemsWidget_->SetTapeMode (tape);
				XmlSettingsManager::Instance ()->setProperty ("TapeMode", tape);
			}
			
			void Aggregator::currentChannelChanged ()
			{
				QModelIndex index = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
				Core::Instance ().CurrentChannelChanged (index, false);
			}
			
			void Aggregator::unreadNumberChanged (int number)
			{
				if (!number ||
						!XmlSettingsManager::Instance ()->
							property ("ShowIconInTray").toBool ())
				{
					Impl_->TrayIcon_->hide ();
					return;
				}
			
				QString tip = tr ("%1 unread messages in %2 channels.")
					.arg (number)
					.arg (Core::Instance ().GetUnreadChannelsNumber ());
				Impl_->TrayIcon_->setToolTip (tip);
				Impl_->TrayIcon_->show ();
			}
			
			void Aggregator::trayIconActivated ()
			{
				emit bringToFront ();
				QModelIndex unread = Core::Instance ().GetUnreadChannelIndex ();
				if (unread.isValid ())
					Impl_->Ui_.Feeds_->setCurrentIndex (unread);
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, LeechCraft::Plugins::Aggregator::Aggregator);

