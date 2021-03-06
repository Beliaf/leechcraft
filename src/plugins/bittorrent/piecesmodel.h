#ifndef PLUGINS_BITTORRENT_PIECESMODEL_H
#define PLUGINS_BITTORRENT_PIECESMODEL_H
#define PLUGINS_BITTORRENT_WIN32_LEAN_AND_MEAN
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <vector>
#include <libtorrent/torrent_handle.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class Core;

			class PiecesModel : public QAbstractItemModel
			{
				Q_OBJECT

				friend class Core;

				QStringList Headers_;
				struct Info
				{
					int Index_;
					libtorrent::partial_piece_info::state_t State_;
					int FinishedBlocks_;
					int TotalBlocks_;

					bool operator== (const Info&) const;
				};
				QList<Info> Pieces_;
			public:
				PiecesModel (QObject *parent = 0);
				virtual ~PiecesModel ();

				virtual int columnCount (const QModelIndex&) const;
				virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
			protected:
				void Clear ();
				void Update (const std::vector<libtorrent::partial_piece_info>&);
			};
		};
	};
};

#endif

