#include <QtGui>
#include <plugininterface/proxy.h>
#include "torrentplugin.h"
#include "core.h"
#include "addtorrent.h"
#include "settingsmanager.h"

void TorrentPlugin::Init ()
{
    Q_INIT_RESOURCE (resources);
    QTranslator *transl = new QTranslator (this);
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_torrent_") + localeName);
    qApp->installTranslator (transl);

	setupUi (this);
	IsShown_ = false;
	SettingsDialog_ = new SettingsDialog (this);
	SettingsDialog_->RegisterObject (SettingsManager::Instance ());
	AddTorrentDialog_ = new AddTorrent (this);
	connect (Core::Instance (), SIGNAL (error (QString)), this, SLOT (showError (QString)));
	connect (Core::Instance (), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (updateTorrentStats ()));
	TorrentView_->setModel (Core::Instance ());
	Core::Instance ()->DoDelayedInit ();
	
	OverallStatsUpdateTimer_ = new QTimer (this);
	connect (OverallStatsUpdateTimer_, SIGNAL (timeout ()), this, SLOT (updateOverallStats ()));
	OverallStatsUpdateTimer_->start (500);

	QFontMetrics fm = fontMetrics ();
	QHeaderView *header = TorrentView_->header ();
	header->resizeSection (Core::ColumnName, fm.width ("thisisanaveragewareztorrentname,right?maybeyes.torrent"));
	header->resizeSection (Core::ColumnDownloaded, fm.width ("_1234.0 KB_"));
	header->resizeSection (Core::ColumnUploaded, fm.width ("_1234.0 KB_"));
	header->resizeSection (Core::ColumnSize, fm.width ("_1234.0 KB_"));
	header->resizeSection (Core::ColumnProgress, fm.width ("___100%___"));
	header->resizeSection (Core::ColumnState, fm.width ("__Downloading__"));
	header->resizeSection (Core::ColumnSP, fm.width ("_123/123_"));
	header->resizeSection (Core::ColumnUSpeed, fm.width ("_1234.0 KB/s_"));
	header->resizeSection (Core::ColumnDSpeed, fm.width ("_1234.0 KB/s_"));
	header->resizeSection (Core::ColumnRemaining, fm.width ("10d 00:00:00"));
}

QString TorrentPlugin::GetName () const
{
	return windowTitle ();
}

QString TorrentPlugin::GetInfo () const
{
	return tr ("BitTorrent client using rb-libtorrent.");
}

QString TorrentPlugin::GetStatusbarMessage () const
{
	return QString ("");
}

IInfo& TorrentPlugin::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t TorrentPlugin::GetID () const
{
	return ID_;
}

QStringList TorrentPlugin::Provides () const
{
	return QStringList ("bittorrent") << "resume";
}

QStringList TorrentPlugin::Needs () const
{
	return QStringList ();
}

QStringList TorrentPlugin::Uses () const
{
	return QStringList ();
}

void TorrentPlugin::SetProvider (QObject*, const QString&)
{
}

void TorrentPlugin::Release ()
{
	Core::Instance ()->Release ();
	SettingsManager::Instance ()->Release ();
}

QIcon TorrentPlugin::GetIcon () const
{
	return windowIcon ();
}

void TorrentPlugin::SetParent (QWidget *w)
{
	setParent (w);
}

void TorrentPlugin::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void TorrentPlugin::ShowBalloonTip ()
{
}

qint64 TorrentPlugin::GetDownloadSpeed () const
{
	return 0;
}

qint64 TorrentPlugin::GetUploadSpeed () const
{
	return 0;
}

void TorrentPlugin::StartAll ()
{
}

void TorrentPlugin::StopAll ()
{
}

void TorrentPlugin::StartAt (ulong)
{
}

void TorrentPlugin::StopAt (ulong)
{
}

void TorrentPlugin::DeleteAt (ulong)
{
}

void TorrentPlugin::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void TorrentPlugin::on_OpenTorrent__triggered ()
{
	AddTorrentDialog_->Reinit ();
	if (AddTorrentDialog_->exec () == QDialog::Rejected)
		return;

	QString filename = AddTorrentDialog_->GetFilename (),
			path = AddTorrentDialog_->GetSavePath ();
	Core::Instance ()->AddFile (filename, path);
}

void TorrentPlugin::on_RemoveTorrent__triggered ()
{
	int row = TorrentView_->currentIndex ().row ();
	if (row == -1)
		return;

	Core::Instance ()->RemoveTorrent (row);
	updateTorrentStats ();
}

void TorrentPlugin::on_Resume__triggered ()
{
	Core::Instance ()->ResumeTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Stop__triggered ()
{
	Core::Instance ()->PauseTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Preferences__triggered ()
{
	SettingsDialog_->show ();
	SettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void TorrentPlugin::on_TorrentView__clicked (const QModelIndex&)
{
	updateTorrentStats ();
}

void TorrentPlugin::on_TorrentView__pressed (const QModelIndex&)
{
	updateTorrentStats ();
}

void TorrentPlugin::setActionsEnabled ()
{
}

void TorrentPlugin::showError (QString e)
{
	qWarning () << e;
	QMessageBox::warning (this, tr ("Error!"), e);
}

void TorrentPlugin::updateTorrentStats ()
{
	QModelIndex index = TorrentView_->currentIndex ();
	if (!index.isValid ())
	{
		LabelState_->setText ("<>");
		LabelTracker_->setText ("<>");
		LabelProgress_->setText ("<>");
		LabelDHTNodesCount_->setText ("<>");
		LabelDownloaded_->setText ("<>");
		LabelTotalSize_->setText ("<>");
		LabelFailed_->setText ("<>");
		LabelConnectedSeeds_->setText ("<>");
		LabelConnectedPeers_->setText ("<>");
		LabelNextAnnounce_->setText ("<>");
		LabelAnnounceInterval_->setText ("<>");
		LabelTotalPieces_->setText ("<>");
		LabelDownloadedPieces_->setText ("<>");
		LabelPieceSize_->setText ("<>");
		LabelDownloadRate_->setText ("<>");
		LabelUploadRate_->setText ("<>");
	}
	else
	{
		TorrentInfo i = Core::Instance ()->GetTorrentStats (index.row ());
		LabelState_->setText (i.State_);
		LabelTracker_->setText (i.Tracker_);
		LabelProgress_->setText (QString::number (i.Progress_ * 100) + "%");
		LabelDHTNodesCount_->setText (QString::number (i.DHTNodesCount_));
		LabelDownloaded_->setText (Proxy::Instance ()->MakePrettySize (i.Downloaded_));
		LabelTotalSize_->setText (Proxy::Instance ()->MakePrettySize (i.TotalSize_));
		LabelFailed_->setText (Proxy::Instance ()->MakePrettySize (i.FailedSize_));
		LabelConnectedPeers_->setText (QString::number (i.ConnectedPeers_));
		LabelConnectedSeeds_->setText (QString::number (i.ConnectedSeeds_));
		LabelNextAnnounce_->setText (i.NextAnnounce_.toString ());
		LabelAnnounceInterval_->setText (i.AnnounceInterval_.toString ());
		LabelTotalPieces_->setText (QString::number (i.TotalPieces_));
		LabelDownloadedPieces_->setText (QString::number (i.DownloadedPieces_));
		LabelPieceSize_->setText (Proxy::Instance ()->MakePrettySize (i.PieceSize_));
		LabelDownloadRate_->setText (Proxy::Instance ()->MakePrettySize (i.DownloadRate_) + tr ("/s"));
		LabelUploadRate_->setText (Proxy::Instance ()->MakePrettySize (i.UploadRate_) + tr ("/s"));
	}
}

void TorrentPlugin::updateOverallStats ()
{
	OverallStats stats = Core::Instance ()->GetOverallStats ();
	LabelTotalDownloadRate_->setText (Proxy::Instance ()->MakePrettySize (static_cast<int> (stats.DownloadRate_)) + tr ("/s"));
	LabelTotalUploadRate_->setText (Proxy::Instance ()->MakePrettySize (static_cast<int> (stats.UploadRate_)) + tr ("/s"));
	LabelTotalDownloaded_->setText (Proxy::Instance ()->MakePrettySize (stats.SessionDownload_));
	LabelTotalUploaded_->setText (Proxy::Instance ()->MakePrettySize (stats.SessionUpload_));
	LabelTotalConnections_->setText (QString::number (stats.NumConnections_));
	LabelUploadConnections_->setText (QString::number (stats.NumUploads_));
	LabelTotalPeers_->setText (QString::number (stats.NumPeers_));
	LabelTotalDHTNodes_->setText (QString::number (stats.NumDHTNodes_));
	LabelDHTTorrents_->setText (QString::number (stats.NumDHTTorrents_));
	LabelListenPort_->setText (QString::number (stats.ListenPort_));
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);
