#include "tagslineedit.h"
#include <QtDebug>
#include <QTimer>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include "tagscompletionmodel.h"
#include "tagscompleter.h"

using namespace LeechCraft::Util;

TagsLineEdit::TagsLineEdit (QWidget *parent)
: QLineEdit (parent)
, Completer_ (0)
{

}

void TagsLineEdit::AddSelector ()
{
	CategorySelector_.reset (new CategorySelector (parentWidget ()));
	CategorySelector_->hide ();

	QAbstractItemModel *model = Completer_->model ();

	connect (model,
			SIGNAL (tagsUpdated (const QStringList&)),
			this,
			SLOT (handleTagsUpdated (const QStringList&)));

	QStringList initialTags;
	for (int i = 0; i < model->rowCount (); ++i)
		initialTags << model->data (model->index (i, 0)).toString ();
	handleTagsUpdated (initialTags);

	connect (CategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			this,
			SLOT (handleSelectionChanged (const QStringList&)));

	connect (this,
			SIGNAL (textChanged (const QString&)),
			CategorySelector_.get (),
			SLOT (lineTextChanged (const QString&)));
}

void TagsLineEdit::insertTag (const QString& completion)
{
	if (Completer_->widget () != this)
		return;

	QString wtext = text ();
	if (completion.startsWith (wtext))
		wtext.clear ();
	int pos = wtext.lastIndexOf (' ');
	wtext = wtext.left (pos).append (' ').append (completion);
	wtext = wtext.simplified ();
	setText (wtext);
}

void TagsLineEdit::handleTagsUpdated (const QStringList& tags)
{
	CategorySelector_->SetPossibleSelections (tags);
}

void TagsLineEdit::handleSelectionChanged (const QStringList& tags)
{
	setText (tags.join (" "));
}

void TagsLineEdit::keyPressEvent (QKeyEvent *e)
{
	if (Completer_ && Completer_->popup ()->isVisible ())
		switch (e->key ())
		{
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
			case Qt::Key_Backtab:
				e->ignore ();
				return;
			default:
				break;
		}

	QLineEdit::keyPressEvent (e);

	bool cos = e->modifiers () & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!Completer_ || (cos && e->text ().isEmpty ()))
		return;

	QString completionPrefix = textUnderCursor ();
	Completer_->setCompletionPrefix (completionPrefix);
	Completer_->popup ()->
		setCurrentIndex (Completer_->completionModel ()->index (0, 0));
	Completer_->complete ();
}

void TagsLineEdit::focusInEvent (QFocusEvent *e)
{
	if (Completer_)
		Completer_->setWidget (this);
	QLineEdit::focusInEvent (e);
}

void TagsLineEdit::contextMenuEvent (QContextMenuEvent *e)
{
	if (!CategorySelector_.get ())
	{
		QLineEdit::contextMenuEvent (e);
		return;
	}

	CategorySelector_->move (e->globalPos ());
	CategorySelector_->show ();
}

void TagsLineEdit::SetCompleter (TagsCompleter *c)
{
	if (Completer_)
		disconnect (Completer_,
				0,
				this,
				0);

	Completer_ = c;

	if (!Completer_)
		return;

	Completer_->setWidget (this);
	Completer_->setCompletionMode (QCompleter::PopupCompletion);
	connect (Completer_,
			SIGNAL (activated (const QString&)),
			this,
			SLOT (insertTag (const QString&)));
}

QString TagsLineEdit::textUnderCursor () const
{
	QRegExp rx (";\\s*");
	QString wtext = text ();
	int pos = cursorPosition () - 1;
	int last = wtext.indexOf (rx, pos);
	int first = wtext.lastIndexOf (rx, pos);
	if (first == -1)
		first = 0;
	if (last == -1)
		last = wtext.size ();
	return wtext.mid (first, last - first);
}

