#ifndef PLUGINS_POSHUKU_BROWSERWIDGET_H
#define PLUGINS_POSHUKU_BROWSERWIDGET_H
#include <boost/shared_ptr.hpp>
#include <QWidget>
#include <interfaces/imultitabs.h>
#include <interfaces/iwebbrowser.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/structures.h>
#include "ui_browserwidget.h"

class QToolBar;
class QMovie;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FindDialog;
			class PasswordRemember;

			class BrowserWidget : public QWidget
								, public IWebWidget
								, public IMultiTabsWidget
			{
				Q_OBJECT
				Q_INTERFACES (IWebWidget IMultiTabsWidget)

				Ui::BrowserWidget Ui_;

				QToolBar *ToolBar_;
				QAction *Add2Favorites_;
				QAction *Find_;
				QAction *Print_;
				QAction *PrintPreview_;
				QAction *ScreenSave_;
				QAction *ViewSources_;
				QAction *NewTab_;
				QAction *CloseTab_;
				QAction *ZoomIn_;
				QAction *ZoomOut_;
				QAction *ZoomReset_;
				QAction *ImportXbel_;
				QAction *ExportXbel_;
				QAction *Cut_;
				QAction *Copy_;
				QAction *Paste_;
				QAction *Back_;
				QAction *Forward_;
				QAction *Reload_;
				QAction *Stop_;
				QAction *RecentlyClosedAction_;
				QPoint OnLoadPos_;
				boost::shared_ptr<QMovie> Loading_;
				QMenu *RecentlyClosed_;
				QMenu *ExternalLinks_;
				FindDialog *FindDialog_;
				PasswordRemember *RememberDialog_;
				bool HtmlMode_;
				bool Own_;
			public:
				enum Actions
				{
					EAAdd2Favorites_,
					EAFind_,
					EAPrint_,
					EAPrintPreview_,
					EAScreenSave_,
					EAViewSources_,
					EANewTab_,
					EACloseTab_,
					EAZoomIn_,
					EAZoomOut_,
					EAZoomReset_,
					EAImportXbel_,
					EAExportXbel_,
					EACut_,
					EACopy_,
					EAPaste_,
					EABack_,
					EAForward_,
					EAReload_,
					EAStop_,
					EARecentlyClosedAction_
				};

				BrowserWidget (QWidget* = 0);
				virtual ~BrowserWidget ();

				void Deown ();
				void InitShortcuts ();

				void SetUnclosers (const QList<QAction*>&);
				CustomWebView* GetView () const;
				void SetURL (const QUrl&);

				void Load (const QString&);
				void SetHtml (const QString&, const QUrl& = QUrl ());
				QWidget* Widget ();

				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;

				void Remove ();
				QToolBar* GetToolBar () const;

				void SetOnLoadScrollPoint (const QPoint&);
			private:
				void PrintImpl (bool, QWebFrame*);
			private slots:
				void handleIconChanged ();
				void handleStatusBarMessage (const QString&);
				void on_URLEdit__returnPressed ();
				void handleAdd2Favorites ();
				void handleFind ();
				void findText (const QString&, QWebPage::FindFlags);
				void handleViewPrint (QWebFrame*);
				void handlePrinting ();
				void handlePrintingWithPreview ();
				void handleScreenSave ();
				void handleViewSources ();
				void handleNewTab ();
				void focusLineEdit ();
				void handleNewUnclose (QAction*);
				void handleUncloseDestroyed ();
				void updateTooltip ();
				void enableActions ();
				void setupLoading ();
				void handleEntityAction ();
				void handleLoadFinished ();
				void handleLoadProgress (int);
			signals:
				void titleChanged (const QString&);
				void urlChanged (const QString&);
				void iconChanged (const QIcon&);
				void needToClose ();
				void tooltipChanged (QWidget*);
				void addToFavorites (const QString&, const QString&);
				void statusBarChanged (const QString&);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void couldHandle (const LeechCraft::DownloadEntity&, bool*);
			};
		};
	};
};

#endif

