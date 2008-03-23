#include <QtDebug>
#include "treeitem.h"
#include "channelsmodel.h"
#include "channel.h"
#include "feed.h"

ChannelsModel::ChannelsModel (QObject *parent)
: QAbstractItemModel (parent)
{
    QVariantList roots;
    roots << tr ("Feed") << tr ("Last build");
    RootItem_ = new TreeItem (roots);
}

ChannelsModel::~ChannelsModel ()
{
    delete RootItem_;
}

int ChannelsModel::columnCount (const QModelIndex& parent) const
{
    if (parent.isValid ())
        return static_cast<TreeItem*> (parent.internalPointer ())->ColumnCount ();
    else
        return RootItem_->ColumnCount();
}

QVariant ChannelsModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();
    if (role != Qt::DisplayRole)
        return QVariant ();

    return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
}

Qt::ItemFlags ChannelsModel::flags (const QModelIndex& index) const
{
    if (!index.isValid ())
        return 0;
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ChannelsModel::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return RootItem_->Data (column);
    else
        return QVariant ();
}

QModelIndex ChannelsModel::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    TreeItem *parentItem;
    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    TreeItem *childItem = parentItem->Child (row);
    if (childItem)
        return createIndex (row, column, childItem);
    else
        return QModelIndex ();
}

QModelIndex ChannelsModel::parent (const QModelIndex& index) const
{
    if (!index.isValid ())
        return QModelIndex ();

    TreeItem *childItem = static_cast<TreeItem*> (index.internalPointer ());
    TreeItem *parentItem = childItem->Parent ();

    if (parentItem == RootItem_)
        return QModelIndex ();
    else
        return createIndex (parentItem->Row (), 0, parentItem);
}

int ChannelsModel::rowCount (const QModelIndex& parent) const
{
    if (parent.column () > 0)
        return 0;

    TreeItem *parentItem;
    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    return parentItem->ChildCount ();
}

void ChannelsModel::AddFeed (const Feed& feed)
{
    std::vector<boost::shared_ptr<Channel> > channels = feed.Channels_;
    beginInsertRows (QModelIndex (), rowCount (), rowCount () + channels.size () - 1);
    for (int i = 0; i < channels.size (); ++i)
    {
        QList<QVariant> data;
        boost::shared_ptr<Channel> current = channels.at (i);
        data << current->Title_ << current->LastBuild_;
        TreeItem *channelItem = new TreeItem (data, RootItem_);
        RootItem_->AppendChild (channelItem);
        Channel2TreeItem_ [current] = channelItem;
        TreeItem2Channel_ [channelItem] = current;
    }
    endInsertRows ();
}

void ChannelsModel::Update (const std::vector<boost::shared_ptr<Channel> >& channels)
{
    QList<boost::shared_ptr<Channel> > channelswh = Channel2TreeItem_.keys ();
    for (int i = 0; i < channels.size (); ++i)
    {
        bool found = false;
        for (int j = 0; j < channelswh.size (); ++j)
            if (*channels.at (i) == *channelswh.at (j))
                found = true;
        if (found)
            continue;
        QList<QVariant> data;
        boost::shared_ptr<Channel> current = channels.at (i);
        data << current->Title_ << current->LastBuild_;
        TreeItem *channelItem = new TreeItem (data, RootItem_);
        RootItem_->AppendChild (channelItem);
        Channel2TreeItem_ [current] = channelItem;
        TreeItem2Channel_ [channelItem] = current;
    }
}

void ChannelsModel::UpdateTimestamp (const boost::shared_ptr<Channel>& channel)
{
    if (!Channel2TreeItem_.contains (channel))
    {
        qWarning () << Q_FUNC_INFO << channel << "not found";
        return;
    }
    TreeItem *item = Channel2TreeItem_ [channel];
    item->ModifyData (1, channel->LastBuild_);
    int pos = RootItem_->ChildPosition (item);
    emit dataChanged (index (pos, 1), index (pos, 1));
}

boost::shared_ptr<Channel> ChannelsModel::GetChannelForIndex (const QModelIndex& index) const
{
    if (index.isValid ())
        return TreeItem2Channel_ [static_cast<TreeItem*> (index.internalPointer ())];
    else
        return boost::shared_ptr<Channel> ();
}
