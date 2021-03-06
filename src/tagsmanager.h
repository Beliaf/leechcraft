#ifndef TAGSMANAGER_H
#define TAGSMANAGER_H
#include "interfaces/iinfo.h"
#include <QAbstractItemModel>
#include <QMap>
#include <QUuid>
#include <QString>
#include <QMetaType>

namespace LeechCraft
{
	class TagsManager : public QAbstractItemModel
					  , public ITagsManager
	{
		Q_OBJECT
		Q_INTERFACES (ITagsManager);

		TagsManager ();
	public:
		typedef QMap<QUuid, QString> TagsDictionary_t;
	private:
		TagsDictionary_t Tags_;
	public:
		static TagsManager& Instance ();
		virtual ~TagsManager ();

		int columnCount (const QModelIndex&) const;
		QVariant data (const QModelIndex&, int) const;
		QModelIndex index (int, int, const QModelIndex&) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex&) const;

		tag_id GetID (const QString&);
		QString GetTag (tag_id) const;
		QStringList Split (const QString&) const;
		QString Join (const QStringList&) const;
		QAbstractItemModel* GetModel ();

		void RemoveTag (const QModelIndex&);
		void SetTag (const QModelIndex&, const QString&);
	private:
		tag_id InsertTag (const QString&);
		QStringList GetAllTags () const;
		void ReadSettings ();
		void WriteSettings () const;
	signals:
		void tagsUpdated (const QStringList&);
	};
};

Q_DECLARE_METATYPE (LeechCraft::TagsManager::TagsDictionary_t);

#endif

