#include "favoritesmodel.h"
#include <algorithm>
#include <QTimer>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "filtermodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			bool FavoritesModel::FavoritesItem::operator== (const FavoritesModel::FavoritesItem& item) const
			{
				return Title_ == item.Title_ &&
					URL_ == item.URL_ &&
					Tags_ == item.Tags_;
			}
			
			FavoritesModel::FavoritesModel (QObject *parent)
			: QAbstractItemModel (parent)
			{
				ItemHeaders_ << tr ("Title")
					<< tr ("URL")
					<< tr ("Tags");
				QTimer::singleShot (0, this, SLOT (loadData ()));
			}
			
			FavoritesModel::~FavoritesModel ()
			{
			}
			
			int FavoritesModel::columnCount (const QModelIndex&) const
			{
				return ItemHeaders_.size ();
			}
			
			QVariant FavoritesModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				switch (role)
				{
					case Qt::DisplayRole:
						switch (index.column ())
						{
							case ColumnTitle:
								return Items_ [index.row ()].Title_;
							case ColumnURL:
								return Items_ [index.row ()].URL_;
							case ColumnTags:
								return Core::Instance ().GetProxy ()->
									GetTagsManager ()->Join (GetVisibleTags (index.row ()));
							default:
								return QVariant ();
						}
					case TagsRole:
						return Core::Instance ().GetProxy ()->
							GetTagsManager ()->Join (GetVisibleTags (index.row ()));
					default:
						return QVariant ();
				}
			}
			
			Qt::ItemFlags FavoritesModel::flags (const QModelIndex& index) const
			{
				Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
				if (index.column () == ColumnTags)
					result |= Qt::ItemIsEditable;
				return result;
			}
			
			QVariant FavoritesModel::headerData (int column, Qt::Orientation orient,
					int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return ItemHeaders_.at (column);
				else
					return QVariant ();
			}
			
			QModelIndex FavoritesModel::index (int row, int column,
					const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex FavoritesModel::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int FavoritesModel::rowCount (const QModelIndex& index) const
			{
				return index.isValid () ? 0 : Items_.size ();
			}
			
			/** The passed value is a string list with user-visible tags.
			 */
			bool FavoritesModel::setData (const QModelIndex& index,
					const QVariant& value, int)
			{
				if (index.column () != ColumnTags)
					return false;
			
				QStringList userTags = value.toStringList ();
				Items_ [index.row ()].Tags_.clear ();
				Q_FOREACH (QString ut, userTags) 
					Items_ [index.row ()].Tags_.append (Core::Instance ().GetProxy ()->
							GetTagsManager ()->GetID (ut));
				Core::Instance ().GetStorageBackend ()->UpdateFavorites (Items_ [index.row ()]);
				return true;
			}
			
			bool FavoritesModel::AddItem (const QString& title, const QString& url,
				   const QStringList& visibleTags)
			{
				QStringList tags;
				Q_FOREACH (QString vt, visibleTags)
					tags << Core::Instance ().GetProxy ()->GetTagsManager ()->GetID (vt);
				FavoritesItem item =
				{
					title,
					url,
					tags
				};
			
				try
				{
					Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					emit error (tr ("Failed to add<br />%1<br />to Favorites, seems "
								"like such title is already used.").arg (title));
					return false;
				}
				return true;
			}
			
			const FavoritesModel::items_t& FavoritesModel::GetItems () const
			{
				return Items_;
			}
			
			QStringList FavoritesModel::GetVisibleTags (int index) const
			{
				QStringList user;
				Q_FOREACH (QString id, Items_ [index].Tags_)
					user.append (Core::Instance ().GetProxy ()->GetTagsManager ()->
							GetTag (id));
				return user;
			}
			
			void FavoritesModel::removeItem (const QModelIndex& index)
			{
				Core::Instance ().GetStorageBackend ()->RemoveFromFavorites (Items_ [index.row ()]);
			}
			
			void FavoritesModel::handleItemAdded (const FavoritesModel::FavoritesItem& item)
			{
				beginInsertRows (QModelIndex (), 0, 0);
				Items_.push_back (item);
				endInsertRows ();
			}
			
			namespace
			{
				struct ItemFinder
				{
					const QString& URL_;
			
					ItemFinder (const QString& url)
					: URL_ (url)
					{
					}
			
					bool operator() (const FavoritesModel::FavoritesItem& item) const
					{
						return item.URL_ == URL_;
					}
				};
			};
			
			void FavoritesModel::handleItemUpdated (const FavoritesModel::FavoritesItem& item)
			{
				items_t::iterator pos =
					std::find_if (Items_.begin (), Items_.end (), ItemFinder (item.URL_));
				if (pos == Items_.end ())
				{
					qWarning () << Q_FUNC_INFO << "not found updated item";
					return;
				}
			
				*pos = item;
			
				int n = std::distance (Items_.begin (), pos);
			
				emit dataChanged (index (n, 2), index (n, 2));
			}
			
			void FavoritesModel::handleItemRemoved (const FavoritesModel::FavoritesItem& item)
			{
				items_t::iterator pos =
					std::find (Items_.begin (), Items_.end (), item);
				if (pos == Items_.end ())
				{
					qWarning () << Q_FUNC_INFO << "not found removed item";
					return;
				}
			
				int n = std::distance (Items_.begin (), pos);
				beginRemoveRows (QModelIndex (), n, n);
				Items_.erase (pos);
				endRemoveRows ();
			}
			
			void FavoritesModel::loadData ()
			{
				items_t items;
				Core::Instance ().GetStorageBackend ()->LoadFavorites (items);
			
				if (!items.size ())
					return;
			
				beginInsertRows (QModelIndex (), 0, items.size () - 1);
				for (items_t::const_reverse_iterator i = items.rbegin (),
						end = items.rend (); i != end; ++i)
					Items_.push_back (*i);
				endInsertRows ();
			}
		};
	};
};

