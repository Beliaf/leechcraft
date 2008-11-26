#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H
#include <deque>
#include <QAbstractItemModel>
#include <QStringList>
#include <QDateTime>

class HistoryModel : public QAbstractItemModel
{
	Q_OBJECT

	QStringList ItemHeaders_;

	struct HistoryItem
	{
		QString Title_;
		QDateTime DateTime_;
		QString URL_;
	};

	std::deque<HistoryItem> Items_;
public:
	enum Columns
	{
		ColumnTitle
		, ColumnDate
		, ColumnURL
	};

	HistoryModel (QObject* = 0);
	virtual ~HistoryModel ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation,
			int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int,
			const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	void AddItem (const QString&, const QString&, const QDateTime&);
private:
	void SaveData () const;
	void LoadData ();
};

#endif
