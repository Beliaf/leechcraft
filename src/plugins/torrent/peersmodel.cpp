#include <QtDebug>
#include <numeric>
#include <plugininterface/proxy.h>
#include "core.h"
#include "peersmodel.h"

PeersModel::PeersModel (QObject *parent)
: QAbstractItemModel (parent)
, CurrentTorrent_ (-1)
{
    Headers_ << tr ("IP")
        << tr ("Drate")
        << tr ("Urate")
        << tr ("Downloaded")
        << tr ("Uploaded")
        << tr ("Client")
        << tr ("Available pieces")
        << tr ("LB")
        << tr ("Last active")
        << tr ("Hashfails")
        << tr ("Failcount")
        << tr ("Progress");
}

PeersModel::~PeersModel ()
{
}

int PeersModel::columnCount (const QModelIndex& index) const
{
    return Headers_.size ();
}

QVariant PeersModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid () || (role != Qt::DisplayRole && role != SortRole))
        return QVariant ();

    const std::vector<bool>* localPieces = Core::Instance ()->GetLocalPieces (CurrentTorrent_);

    int i = index.row ();

    switch (index.column ())
    {
        case 0:
            return Peers_.at (index.row ()).IP_;
        case 1:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).DSpeed_) + tr ("/s");
            else if (role == SortRole)
                return Peers_.at (i).DSpeed_;
            else
                return QVariant ();
        case 2:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).USpeed_) + tr ("/s");
            else if (role == SortRole)
                return Peers_.at (i).USpeed_;
            else
                return QVariant ();
        case 3:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).Downloaded_);
            else if (role == SortRole)
                return Peers_.at (i).Downloaded_;
            else
                return QVariant ();
        case 4:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).Uploaded_);
            else if (role == SortRole)
                return Peers_.at (i).Uploaded_;
            else
                return QVariant ();
        case 5:
            return Peers_.at (i).Client_;
        case 6:
            if (localPieces)
            {
                int remoteNum = std::accumulate (Peers_.at (i).Pieces_.begin (), Peers_.at (i).Pieces_.end (), 0),
                    remoteHasWeDont = 0;
                for (int j = 0; j < localPieces->size (); ++j)
                    remoteHasWeDont += (Peers_.at (i).Pieces_ [j] && !(*localPieces) [j]);
                return tr ("%1, %2 we don't have").arg (remoteNum).arg (remoteHasWeDont);
            }
            else
                return "";
        case 7:
            return static_cast<quint64> (Peers_.at (i).LoadBalancing_);
        case 8:
            return Peers_.at (i).LastActive_.toString ("mm:ss.zzz");
        case 9:
            return Peers_.at (i).Hashfails_;
        case 10:
            return Peers_.at (i).Failcount_;
        case 11:
            if (Peers_.at (i).DownloadingPiece_ >= 0)
                return tr ("Piece %1, block %2, %3 of %4 bytes").
                        arg (Peers_.at (i).DownloadingPiece_).
                        arg (Peers_.at (i).DownloadingBlock_).
                        arg (Peers_.at (i).DownloadingProgress_).
                        arg (Peers_.at (i).DownloadingTotal_);
            else
                return tr ("Not downloading");
        default:
            return "Unhandled column";
    }
}

Qt::ItemFlags PeersModel::flags (const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool PeersModel::hasChildren (const QModelIndex& index) const
{
    return !index.isValid ();
}

QModelIndex PeersModel::index (int row, int column, const QModelIndex&) const
{
    if (!hasIndex (row, column))
        return QModelIndex ();

    return createIndex (row, column);
} 

QVariant PeersModel::headerData (int column, Qt::Orientation orient, int role) const
{
    if (role != Qt::DisplayRole || orient != Qt::Horizontal)
        return QVariant ();

    return Headers_.at (column);
}

QModelIndex PeersModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int PeersModel::rowCount (const QModelIndex& index) const
{
    Peers_.size ();
}

void PeersModel::Clear ()
{
    if (!Peers_.size ())
        return;

    beginRemoveRows (QModelIndex (), 0, Peers_.size () - 1);
    Peers_.clear ();
    endRemoveRows ();
}

void PeersModel::Update (const QList<PeerInfo>& peers, int torrent)
{
    CurrentTorrent_ = torrent;
    QList<PeerInfo> peers2insert;
    QMap<QString, int> IP2position;

    for (int i = 0; i < Peers_.size (); ++i)
        IP2position [Peers_.at (i).IP_] = i;

    for (int i = 0; i < peers.size (); ++i)
    {
        PeerInfo pi = peers.at (i);

        int found = false;
        for (int j = 0; j < Peers_.size (); ++j)
            if (Peers_.at (j).IP_ == pi.IP_)
            {
                found = true;
                Peers_ [j] = pi;
                IP2position.remove (pi.IP_);
                emit dataChanged (index (j, 1), index (j, 11));
                break;
            }

        if (found)
            continue;

        peers2insert << pi;
    }

    QList<int> values = IP2position.values ();
    qSort (values.begin (), values.end (), qGreater<int> ());
    for (int i = 0; i < values.size (); ++i)
    {
        beginRemoveRows (QModelIndex (), values.at (i), values.at (i));
        Peers_.removeAt (values.at (i));
        endRemoveRows ();
    }

    if (peers2insert.size ())
    {
        beginInsertRows (QModelIndex (), Peers_.size (), Peers_.size () + peers2insert.size () - 1);
        Peers_ += peers2insert;
        endInsertRows ();
    }
}
