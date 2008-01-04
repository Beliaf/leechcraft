#ifndef CORE_H
#define CORE_H
#include <QAbstractListModel>
#include <QList>
#include <QMultiMap>
#include <QFile>
#include <QPair>
#include "common.h"
#include "interfaces/interfaces.h"

class QString;
class PluginManager;
class QMainWindow;
class PluginInfo;

class Core : public QAbstractTableModel
{
	Q_OBJECT

	QList<int> TasksIDPool_;
	PluginManager *PluginManager_;
	QMainWindow *ReallyMainWindow_;
public:
	Core (QObject *parent = 0);
	~Core ();

	void SetReallyMainWindow (QMainWindow*);
	QMainWindow *GetReallyMainWindow ();

	void DoDelayedInit ();
	void InitTask (const QString&);
	bool ShowPlugin (IInfo::ID_t);
	
	QPair<qint64, qint64> GetSpeeds () const;

	virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
	virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
	virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
	virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex& index) const;
	virtual bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	virtual bool removeRows (int pos, int rows, const QModelIndex& parent = QModelIndex ());
	virtual QModelIndex parent (const QModelIndex& index = QModelIndex ());
private slots:
	void invalidate (unsigned int);
private:
	void PreparePools ();
	void FetchPlugins ();

	QVariant GetTaskData (int, int) const;
signals:
	void throwError (QString, Errors::Severity);
	void pushTask (const QString&, int);
	void gotPlugin (const PluginInfo*);
};

#endif
