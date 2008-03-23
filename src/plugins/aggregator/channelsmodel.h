#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include <vector>

class TreeItem;
class Channel;
class Feed;

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT

    TreeItem *RootItem_;
    QMap<boost::shared_ptr<Channel>, TreeItem*> Channel2TreeItem_;
    QMap<TreeItem*, boost::shared_ptr<Channel> > TreeItem2Channel_;
public:
    ChannelsModel (QObject *parent = 0);
    virtual ~ChannelsModel ();

    virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    void AddFeed (const Feed&);
    void Update (const std::vector<boost::shared_ptr<Channel> >&);
    void UpdateTimestamp (const boost::shared_ptr<Channel>&);
    boost::shared_ptr<Channel> GetChannelForIndex (const QModelIndex&) const;
};

#endif
