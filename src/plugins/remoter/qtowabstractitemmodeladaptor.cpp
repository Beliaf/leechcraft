#include "qtowabstractitemmodeladaptor.h"
#include <WDate>
#include <QDate>
#include <QUrl>
#include <QtDebug>
#include <plugininterface/treeitem.h>

QToWAbstractItemModelAdaptor::QToWAbstractItemModelAdaptor (QAbstractItemModel *model,
		WObject *parent)
: Wt::WAbstractItemModel (parent)
, Model_ (model)
, Root_ (new TreeItem (QVariantList ()))
{
	Indexes_ [Root_] = Wt::WModelIndex ();

	connect (Model_,
			SIGNAL (columnsAboutToBeInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsAboutToBeInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (columnsAboutToBeRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsAboutToBeRemoved (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (columnsInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (columnsRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsRemoved (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (reDataChanged (const QModelIndex&, const QModelIndex&)));
	connect (Model_,
			SIGNAL (headerDataChanged (Qt::Orientation, int, int)),
			this,
			SLOT (reHeaderDataChanged (Qt::Orientation, int, int)));
	connect (Model_,
			SIGNAL (layoutAboutToBeChanged ()),
			this,
			SLOT (reLayoutAboutToBeChanged ()));
	connect (Model_,
			SIGNAL (layoutChanged ()),
			this,
			SLOT (reLayoutChanged ()));
	connect (Model_,
			SIGNAL (modelAboutToBeReset ()),
			this,
			SLOT (reModelAboutToBeReset ()));
	connect (Model_,
			SIGNAL (modelReset ()),
			this,
			SLOT (reModelReset ()));
	connect (Model_,
			SIGNAL (rowsAboutToBeInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsAboutToBeInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (rowsAboutToBeRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsAboutToBeRemoved (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (rowsInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsRemoved (const QModelIndex&, int, int)));
}

QToWAbstractItemModelAdaptor::~QToWAbstractItemModelAdaptor ()
{
}

int QToWAbstractItemModelAdaptor::columnCount (const Wt::WModelIndex& parent) const
{
	return Model_->columnCount (Convert (parent));
}

int QToWAbstractItemModelAdaptor::rowCount (const Wt::WModelIndex& parent) const
{
	return Model_->rowCount (Convert (parent));
}

int QToWAbstractItemModelAdaptor::flags (const Wt::WModelIndex& index) const
{
	Qt::ItemFlags flags = Model_->flags (Convert (index));
	Wt::ItemFlag result;
	if (flags & Qt::ItemIsSelectable)
		result = static_cast<Wt::ItemFlag> (result | Wt::ItemIsSelectable);
	if (flags & Qt::ItemIsEditable)
		result = static_cast<Wt::ItemFlag> (result | Wt::ItemIsEditable);
	if (flags & Qt::ItemIsUserCheckable)
		result = static_cast<Wt::ItemFlag> (result | Wt::ItemIsUserCheckable);
	return result;
}

bool QToWAbstractItemModelAdaptor::hasChildren (const Wt::WModelIndex& index) const
{
	return Model_->hasChildren (Convert (index));
}

Wt::WModelIndex QToWAbstractItemModelAdaptor::parent (const Wt::WModelIndex& i) const
{
	return Indexes_ [static_cast<TreeItem*> (i.internalPointer ())->Parent ()];
}

boost::any QToWAbstractItemModelAdaptor::data (const Wt::WModelIndex& i, int role) const
{
	Qt::ItemDataRole idr;
	switch (static_cast<Wt::ItemDataRole> (role))
	{
		case Wt::DisplayRole:
			idr = Qt::DisplayRole;
			break;
		case Wt::EditRole:
			idr = Qt::EditRole;
			break;
		case Wt::CheckStateRole:
			idr = Qt::CheckStateRole;
			break;
		case Wt::ToolTipRole:
			idr = Qt::ToolTipRole;
			break;
		// TODO implement on-the-fly icon rewriting.
		default:
			return boost::any ();
	}

	return Convert (Model_->data (Convert (i), idr));
}

boost::any QToWAbstractItemModelAdaptor::headerData (int section,
		Wt::Orientation orient, int role) const
{
	Qt::ItemDataRole idr;
	switch (static_cast<Wt::ItemDataRole> (role))
	{
		case Wt::DisplayRole:
			idr = Qt::DisplayRole;
			break;
		case Wt::EditRole:
			idr = Qt::EditRole;
			break;
		case Wt::CheckStateRole:
			idr = Qt::CheckStateRole;
			break;
		case Wt::ToolTipRole:
			idr = Qt::ToolTipRole;
			break;
		// TODO implement on-the-fly icon rewriting.
		default:
			return boost::any ();
	}

	return Convert (Model_->headerData (section,
				(orient == Wt::Horizontal ? Qt::Horizontal : Qt::Vertical),
				idr));
}

Wt::WModelIndex QToWAbstractItemModelAdaptor::index (int row, int column,
		const Wt::WModelIndex& pi) const
{
	TreeItem *parent;
	if (pi.isValid ())
		parent = static_cast<TreeItem*> (pi.internalPointer ());
	else
		parent = Root_;

	TreeItem *item = new TreeItem (QVariantList (), parent);
	parent->AppendChild (item);

	Wt::WModelIndex result = createIndex (row, column, item);
	Indexes_ [item] = result;
	return result;
}

void QToWAbstractItemModelAdaptor::reColumnsAboutToBeInserted (const QModelIndex& index,
		int from, int to)
{
	columnsAboutToBeInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reColumnsAboutToBeRemoved (const QModelIndex& index,
		int from, int to)
{
	columnsAboutToBeRemoved (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reColumnsInserted (const QModelIndex& index,
		int from, int to)
{
	columnsInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reColumnsRemoved (const QModelIndex& index,
		int from, int to)
{
	columnsRemoved (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reDataChanged (const QModelIndex& from,
		const QModelIndex& to)
{
	dataChanged (Convert (from), Convert (to));
}

void QToWAbstractItemModelAdaptor::reHeaderDataChanged (Qt::Orientation orient,
		int from, int to)
{
	headerDataChanged ((orient == Qt::Horizontal ? Wt::Horizontal : Wt::Vertical),
			from, to);
}

void QToWAbstractItemModelAdaptor::reLayoutAboutToBeChanged ()
{
	layoutAboutToBeChanged ();
}

void QToWAbstractItemModelAdaptor::reLayoutChanged ()
{
	layoutChanged ();
}

void QToWAbstractItemModelAdaptor::reModelAboutToBeReset ()
{
//	No corresponding signal, let's emulate
//	modelAboutToBeReset ();
}

void QToWAbstractItemModelAdaptor::reModelReset ()
{
	int frow = rowCount () - 1;
	rowsAboutToBeRemoved (Wt::WModelIndex (), 0, frow);
	rowsRemoved (Wt::WModelIndex (), 0, frow);
//	modelReset ();
}

void QToWAbstractItemModelAdaptor::reRowsAboutToBeInserted (const QModelIndex& index,
		int from, int to)
{
	rowsAboutToBeInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reRowsAboutToBeRemoved (const QModelIndex& index,
		int from, int to)
{
	rowsAboutToBeRemoved (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reRowsInserted (const QModelIndex& index,
		int from, int to)
{
	rowsInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reRowsRemoved (const QModelIndex& index,
		int from, int to)
{
	rowsRemoved (Convert (index), from, to);
}

boost::any QToWAbstractItemModelAdaptor::Convert (const QVariant& var) const
{
	switch (var.type ())
	{
		case QVariant::Bool:
			return boost::any (var.toBool ());
		case QVariant::Char:
		case QVariant::String:
			{
				std::string str = var.toString ().toUtf8 ().constData ();
				return boost::any (str);
			}
		case QVariant::Date:
			{
				QDate date = var.toDate ();
				Wt::WDate res (date.year (), date.month (), date.day ());
				return boost::any (res);
			}
		case QVariant::Double:
			return boost::any (var.toDouble ());
		case QVariant::Int:
			return boost::any (var.toInt ());
		case QVariant::LongLong:
			return boost::any (var.toLongLong ());
		case QVariant::UInt:
			return boost::any (var.toUInt ());
		case QVariant::ULongLong:
			return boost::any (var.toULongLong ());
		// Unconvertable
		case QVariant::BitArray:
		case QVariant::Bitmap:
		case QVariant::Brush:
		case QVariant::ByteArray:
		case QVariant::Color:
		case QVariant::Cursor:
		case QVariant::DateTime:
		case QVariant::Font:
		case QVariant::Icon:
		case QVariant::Image:
		case QVariant::Invalid:
		case QVariant::KeySequence:
		case QVariant::Line:
		case QVariant::LineF:
		case QVariant::List:
		case QVariant::Locale:
		case QVariant::Map:
		case QVariant::Matrix:
		case QVariant::Transform:
		case QVariant::Palette:
		case QVariant::Pen:
		case QVariant::Pixmap:
		case QVariant::Point:
		case QVariant::PointF:
		case QVariant::Polygon:
		case QVariant::Rect:
		case QVariant::RectF:
		case QVariant::RegExp:
		case QVariant::Region:
		case QVariant::Size:
		case QVariant::SizeF:
		case QVariant::SizePolicy:
		case QVariant::StringList:
		case QVariant::TextFormat:
		case QVariant::TextLength:
		case QVariant::Time:
		case QVariant::Url:
		case QVariant::UserType:
		case QVariant::LastType:
			return boost::any ();
	}
}

QVariant QToWAbstractItemModelAdaptor::Convert (const boost::any& a) const
{
	if (a.type () == typeid (bool))
		return QVariant (boost::any_cast<bool> (a));
	else if (a.type () == typeid (std::string))
		return QVariant (QString::fromStdString (boost::any_cast<std::string> (a)));
	else if (a.type () == typeid (Wt::WString))
		return QVariant (QString::fromStdString (boost::any_cast<Wt::WString> (a).toUTF8 ()));
	else if (a.type () == typeid (Wt::WDate))
	{
		Wt::WDate date = boost::any_cast<Wt::WDate> (a);
		return QVariant (QDate (date.year (), date.month (), date.day ()));
	}
	else if (a.type () == typeid (double) ||
			a.type () == typeid (float))
		return QVariant (boost::any_cast<double> (a));
	else if (a.type () == typeid (char))
		return QVariant (boost::any_cast<char> (a));
	else if (a.type () == typeid (int) ||
			a.type () == typeid (short))
		return QVariant (boost::any_cast<int> (a));
	else if (a.type () == typeid (long long) ||
			a.type () == typeid (long))
		return QVariant (boost::any_cast<long long> (a));
	else if (a.type () == typeid (unsigned int))
		return QVariant (boost::any_cast<unsigned int> (a));
	else if (a.type () == typeid (unsigned long long) ||
			a.type () == typeid (unsigned long))
		return QVariant (boost::any_cast<unsigned long long> (a));
	else
		return boost::any_cast<QVariant> (a);
}

QModelIndex QToWAbstractItemModelAdaptor::Convert (const Wt::WModelIndex& wmi) const
{
	if (!wmi.isValid ())
		return QModelIndex ();

	return Model_->index (wmi.row (), wmi.column (), Convert (wmi.parent ()));
}

Wt::WModelIndex QToWAbstractItemModelAdaptor::Convert (const QModelIndex& qmi) const
{
	if (!qmi.isValid ())
		return Wt::WModelIndex ();

	return index (qmi.row (), qmi.column (), Convert (qmi.parent ()));
}
