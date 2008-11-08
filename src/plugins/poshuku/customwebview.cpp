#include "customwebview.h"
#include "core.h"

CustomWebView::CustomWebView (QWidget *parent)
: QWebView (parent)
{
	connect (this,
			SIGNAL (urlChanged (const QUrl&)),
			this,
			SLOT (remakeURL (const QUrl&)));
}

CustomWebView::~CustomWebView ()
{
}

QWebView* CustomWebView::createWindow (QWebPage::WebWindowType)
{
	return Core::Instance ().MakeWebView ();
}

void CustomWebView::remakeURL (const QUrl& url)
{
	emit urlChanged (url.toString ());
}
