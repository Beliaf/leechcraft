#ifndef PLUGINS_AGGREGATOR_ITEMSFILTERMODEL_H
#define PLUGINS_AGGREGATOR_ITEMSFILTERMODEL_H
#include <QSortFilterProxyModel>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemsFilterModel : public QSortFilterProxyModel
			{
				Q_OBJECT

				bool HideRead_;
				QStringList ItemCategories_;
			public:
				ItemsFilterModel (QObject* = 0);
				virtual ~ItemsFilterModel ();

				void SetHideRead (bool);
			protected:
				virtual bool filterAcceptsRow (int, const QModelIndex&) const;
			public slots:
				void categorySelectionChanged (const QStringList&);
			};
		};
	};
};

#endif

