#ifndef PLUGINS_AGGREGATOR_ITEMMODEL_H
#define PLUGINS_AGGREGATOR_ITEMMODEL_H
#include <deque>
#include <QAbstractItemModel>
#include <QStringList>
#include "item.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemModel : public QAbstractItemModel
			{
				Q_OBJECT

				std::deque<Item_ptr> Items_;
				QStringList ItemHeaders_;
				bool SaveScheduled_;
			public:
				ItemModel (QObject* = 0);
				virtual ~ItemModel ();

				void AddItem (const Item_ptr&);
				void RemoveItem (const QModelIndex&);
				void Activated (const QModelIndex&) const;
				QString GetDescription (const QModelIndex&) const;

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			private:
				void ScheduleSave ();
			public slots:
				void saveSettings ();
			};
		};
	};
};

#endif

