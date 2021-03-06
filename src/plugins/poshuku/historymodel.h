#ifndef PLUGINS_POSHUKU_HISTORYMODEL_H
#define PLUGINS_POSHUKU_HISTORYMODEL_H
#include <deque>
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>
#include <QDateTime>

class QTimer;
class QAction;

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem;
	};

	namespace Plugins
	{
		namespace Poshuku
		{
			struct HistoryItem
			{
				QString Title_;
				QDateTime DateTime_;
				QString URL_;
			};

			typedef std::vector<HistoryItem> history_items_t;

			class HistoryModel : public QAbstractItemModel
			{
				Q_OBJECT

				QTimer *GarbageTimer_;
				Util::TreeItem *RootItem_;
				QAction *FolderIconProxy_;
				QAction *UnknownURLProxy_;
			public:
				enum Columns
				{
					ColumnTitle
					, ColumnURL
					, ColumnDate
				};

				HistoryModel (QObject* = 0);
				virtual ~HistoryModel ();

				int columnCount (const QModelIndex& = QModelIndex ()) const;
				QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				Qt::ItemFlags flags (const QModelIndex&) const;
				QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex& = QModelIndex ()) const;

				void AddItem (const QString&, const QString&, const QDateTime&);
			private:
				void Add (const HistoryItem&);
			private slots:
				void loadData ();
				void handleItemAdded (const HistoryItem&);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::HistoryItem);


#endif

