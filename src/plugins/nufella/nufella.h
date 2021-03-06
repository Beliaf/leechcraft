#ifndef NUFELLA_H
#define NUFELLA_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>

class Nufella : public QObject
			  , public IInfo
			  , public IDownload
			  , public IJobHolder
{
	Q_OBJECT

	Q_INTERFACES (IInfo IDownload IJobHolder)

	std::auto_ptr<QTranslator> Translator_;
public:
	void Init (ICoreProxy_ptr);
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	QIcon GetIcon () const;
	qint64 GetDownloadSpeed () const;
	qint64 GetUploadSpeed () const;
	void StartAll ();
	void StopAll ();
	bool CouldDownload (const LeechCraft::DownloadEntity&) const;
	int AddJob (LeechCraft::DownloadEntity);
	QAbstractItemModel* GetRepresentation () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;
	void ItemSelected (const QModelIndex&);
};

#endif

