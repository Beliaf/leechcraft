#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QNetworkRequest>
#include <QRegExp>
#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
#include <plugininterface/util.h>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"
#include "core.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::Poshuku::Plugins::CleanWeb;
using LeechCraft::Util::Proxy;

namespace
{
	struct FilterFinderBase
	{
		const QString& ID_;
		enum Type
		{
			TName_,
			TFilename_
		};

		FilterFinderBase (const QString& id)
		: ID_ (id)
		{
		}
	};

	template<FilterFinderBase::Type>
		struct FilterFinder;

	template<>
		struct FilterFinder<FilterFinderBase::TName_> : FilterFinderBase
		{
			FilterFinder (const QString& id)
			: FilterFinderBase (id)
			{
			}

			bool operator() (const Filter& f) const
			{
				return f.SD_.Name_ == ID_;
			}
		};

	template<>
		struct FilterFinder<FilterFinderBase::TFilename_> : FilterFinderBase
		{
			FilterFinder (const QString& id)
			: FilterFinderBase (id)
			{
			}

			bool operator() (const Filter& f) const
			{
				return f.SD_.Filename_ == ID_;
			}
		};

	struct LineHandler
	{
		Filter *Filter_;

		LineHandler (Filter *f)
		: Filter_ (f)
		{
		}

		void operator() (const QString& line)
		{
			if (line.size () &&
					line.at (0) == '!')
				return;

			QString actualLine;
			FilterOption f = FilterOption ();
			if (line.indexOf ('$') != -1)
			{
				QStringList splitted = line.split ('$',
						QString::SkipEmptyParts);

				if (splitted.size () != 2)
				{
					qWarning () << Q_FUNC_INFO
						<< splitted.size ()
						<< line;
					return;
				}

				actualLine = splitted.at (0);
				QStringList options = splitted.at (1).split (',',
						QString::SkipEmptyParts);

				if (options.contains ("match-case"))
					f.Case_ = Qt::CaseSensitive;
				Q_FOREACH (QString option, options)
					if (option.startsWith ("domain="))
					{
						QString domain = option.remove (0, 7);
						if (domain.startsWith ('~'))
							f.NotDomains_ << domain.remove (0, 1);
						else
							f.Domains_ << domain;
					}
			}
			else
				actualLine = line;

			bool white = false;
			if (actualLine.startsWith ("@@"))
			{
				actualLine.remove (0, 2);
				white = true;
			}

			if (actualLine.startsWith ('/') && 
					actualLine.endsWith ('/'))
			{
				actualLine = actualLine.mid (1, actualLine.size () - 2);
				f.MatchType_ = FilterOption::MTRegexp_;
			}
			else
			{
				if (actualLine.endsWith ('|'))
				{
					actualLine.remove (0, 1);
					actualLine.prepend ('*');
				}
				else if (actualLine.startsWith ('|'))
				{
					actualLine.chop (1);
					actualLine.append ('*');
				}
				else
				{
					actualLine.prepend ('*');
					actualLine.append ('*');
				}
				actualLine.replace ('?', "\\?");
			}

			if (white)
				Filter_->ExceptionStrings_ << actualLine;
			else
				Filter_->FilterStrings_ << actualLine;

			if (FilterOption () != f)
				Filter_->Options_ [actualLine] = f;

			QRegExp::PatternSyntax syntax = (f.MatchType_ == FilterOption::MTRegexp_ ?
					QRegExp::RegExp : QRegExp::Wildcard);
			Filter_->RegExps_ [actualLine] = QRegExp (actualLine, f.Case_, syntax);
		}
	};
};

FilterOption::FilterOption ()
: Case_ (Qt::CaseInsensitive)
, MatchType_ (MTWildcard_)
{
}

bool LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::operator== (const FilterOption& f1, const FilterOption& f2)
{
	return f1.Case_ == f2.Case_ &&
		f1.MatchType_ == f2.MatchType_ &&
		f1.Domains_ == f2.Domains_ &&
		f1.NotDomains_ == f2.NotDomains_;
}

bool LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::operator!= (const FilterOption& f1, const FilterOption& f2)
{
	return !(f1 == f2);
}

LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::Core::Core ()
{
	HeaderLabels_ << tr ("Name")
		<< tr ("Last updated")
		<< tr ("URL");
	try
	{
		LeechCraft::Util::CreateIfNotExists ("cleanweb");
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< "failed to create the directory"
			<< e.what ();
		return;
	}

	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");
	QFileInfoList infos = home.entryInfoList (QDir::Files | QDir::Readable);
	Q_FOREACH (QFileInfo info, infos)
		Parse (info.absoluteFilePath ());

	ReadSettings ();
	Update ();
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
}

int Core::columnCount (const QModelIndex&) const
{
	return HeaderLabels_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () ||
			role != Qt::DisplayRole)
		return QVariant ();

	int row = index.row ();
	switch (index.column ())
	{
		case 0:
			return Filters_.at (row).SD_.Name_;
		case 1:
			return Filters_.at (row).SD_.LastDateTime_;
		case 2:
			return Filters_.at (row).SD_.URL_.toString ();
		default:
			return QVariant ();
	}
}

QVariant Core::headerData (int section, Qt::Orientation orient, int role) const
{
	if (orient != Qt::Horizontal ||
			role != Qt::DisplayRole)
		return QVariant ();

	return HeaderLabels_.at (section);
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Filters_.size ();
}

bool Core::CouldHandle (const DownloadEntity& e) const
{
	if (e.Entity_.size () > 1024)
		return false;

	QString urlString = QTextCodec::codecForName ("UTF-8")->
		toUnicode (e.Entity_);
	QUrl url (urlString);
	if (url.scheme () == "abp" &&
			url.path () == "subscribe")
	{
		QString name = url.queryItemValue ("title");
		if (std::find_if (Filters_.begin (), Filters_.end (),
					FilterFinder<FilterFinderBase::TName_> (name)) == Filters_.end ())
			return true;
		else
			return false;
	}
	else
		return false;
}

void Core::Handle (DownloadEntity subscr)
{
	QString urlString = QTextCodec::codecForName ("UTF-8")->
		toUnicode (subscr.Entity_);
	QUrl subscrUrl (urlString);
	QUrl url (subscrUrl.queryItemValue ("location"));
	QString subscrName = subscrUrl.queryItemValue ("title");

	if (std::find_if (Filters_.begin (), Filters_.end (),
				FilterFinder<FilterFinderBase::TName_> (subscrName)) != Filters_.end ())
		return;

	Load (url, subscrName);
}

QAbstractItemModel* Core::GetModel ()
{
	return this;
}

void Core::Remove (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	Remove (Filters_ [index.row ()].SD_.Filename_);
}

QNetworkReply* Core::Hook (IHookProxy*,
		QNetworkAccessManager::Operation*,
		QNetworkRequest *req,
		QIODevice**)
{
	if (ShouldReject (*req))
	{
		qDebug () << "rejecting" << req->url ();
		*req = QNetworkRequest ();
	}
	return 0;
}

/** We test each filter until we know that we should reject it or until
 * it gets whitelisted.
 *
 * So, for each filter we first iterate through the whitelist. For each
 * entry in the whitelist:
 * - First, we check if the url's domain ends with a string from a "not
 *   apply" list if it's not empty. If it does, we skip this whitelist
 *   entry and go to the next one, if it doesn't, we continue
 *   processing.
 * - Then, if we continue processing, we check if the url's domain ends
 *   with a string from "apply list", if this list isn't empty. If it
 *   ends, we continue processing, otherwise we skip this whilelist
 *   entry and go to the next one.
 * - Then, we check if the URL matches this exception, either by regexp
 *   or wildcard. If it should be matched only in the beginning or in
 *   the end, then '*' is appended or prepended and exact match is
 *   checked. Otherwise only something is required to match. Please not
 *   that the '*' is prepended by the filter parsing code, not this one.
 *
 * The same is applied to the filter strings.
 */
bool Core::ShouldReject (const QNetworkRequest& req) const
{
	QUrl url = req.url ();
	QString urlStr = url.toString ();
	QString domain = url.host ();
	
	Q_FOREACH (Filter filter, Filters_)
	{
		Q_FOREACH (QString exception, filter.ExceptionStrings_)
			if (Matches (exception, filter, urlStr, domain))
				return false;

		Q_FOREACH (QString filterString, filter.FilterStrings_)
			if (Matches (filterString, filter, urlStr, domain))
				return true;
	}

	return false;
}

#if defined (Q_WS_WIN) || defined (Q_WS_MAC)
// Thanks for this goes to http://www.codeproject.com/KB/string/patmatch.aspx
bool WildcardMatches (const char *pattern, const char *str)
{
    enum State {
        Exact,        // exact match
        Any,        // ?
        AnyRepeat    // *
    };

    const char *s = str;
    const char *p = pattern;
    const char *q = 0;
    int state = 0;

    bool match = true;
    while (match && *p) {
        if (*p == '*') {
            state = AnyRepeat;
            q = p+1;
        } else if (*p == '?') state = Any;
        else state = Exact;

        if (*s == 0) break;

        switch (state) {
            case Exact:
                match = *s == *p;
                s++;
                p++;
                break;

            case Any:
                match = true;
                s++;
                p++;
                break;

            case AnyRepeat:
                match = true;
                s++;

                if (*s == *q) p++;
                break;
        }
    }

    if (state == AnyRepeat) return (*s == *q);
    else if (state == Any) return (*s == *p);
    else return match && (*s == *p);
}
#else
#include <fnmatch.h>

bool WildcardMatches (const char *pat, const char *str)
{
	return !fnmatch (pat, str, 0);
}
#endif

bool Core::Matches (const QString& exception, const Filter& filter,
		const QString& urlStr, const QString& domain) const
{
	FilterOption opt = filter.Options_ [exception];
	if (!opt.NotDomains_.isEmpty ())
	{
		bool shouldFurther = true;
		Q_FOREACH (QString notDomain, opt.NotDomains_)
			if (domain.endsWith (notDomain, opt.Case_))
			{
				shouldFurther = false;
				break;
			}
		if (!shouldFurther)
			return false;
	}

	if (!opt.Domains_.isEmpty ())
	{
		bool shouldFurther = false;
		Q_FOREACH (QString doDomain, opt.Domains_)
			if (domain.endsWith (doDomain, opt.Case_))
			{
				shouldFurther = true;
				break;
			}
		if (!shouldFurther)
			return false;
	}

	if (opt.MatchType_ == FilterOption::MTRegexp_ &&
			filter.RegExps_ [exception].exactMatch (urlStr))
		return true;
	else if (opt.MatchType_ == FilterOption::MTWildcard_)
	{
		if (opt.Case_ == Qt::CaseSensitive)
		{
			if (WildcardMatches (qPrintable (exception), qPrintable (urlStr)))
				return true;
		}
		else
		{
			if (WildcardMatches (qPrintable (exception.toLower ()),
						qPrintable (urlStr.toLower ())))
				return true;
		}
	}
	return false;
}

void Core::HandleProvider (QObject *provider)
{
	if (Downloaders_.contains (provider))
		return;
	
	Downloaders_ << provider;
	connect (provider,
			SIGNAL (jobFinished (int)),
			this,
			SLOT (handleJobFinished (int)));
	connect (provider,
			SIGNAL (jobError (int, IDownload::Error)),
			this,
			SLOT (handleJobError (int, IDownload::Error)));
}


void Core::Parse (const QString& filePath)
{
	QFile file (filePath);
	if (!file.open (QIODevice::ReadOnly))
	{
		qWarning () << Q_FUNC_INFO
			<< "could not open file"
			<< filePath
			<< file.errorString ();
		return;
	}

	QString data = QTextCodec::codecForName ("UTF-8")->
		toUnicode (file.readAll ());
	QStringList rawLines = data.split ('\n', QString::SkipEmptyParts);
	if (rawLines.size ())
		rawLines.removeAt (0);
	QStringList lines;
	std::transform (rawLines.begin (), rawLines.end (),
			std::back_inserter (lines),
			boost::bind (&QString::trimmed,
				_1));

	Filter f;
	std::for_each (lines.begin (), lines.end (),
			LineHandler (&f));

	f.SD_.Filename_ = QFileInfo (filePath).fileName ();

	beginInsertRows (QModelIndex (), Filters_.size (), Filters_.size ());
	Filters_ << f;
	endInsertRows ();
}

void Core::Update ()
{
	if (!XmlSettingsManager::Instance ()->
			property ("Autoupdate").toBool ())
		return;

	QDateTime current = QDateTime::currentDateTime ();
	int days = XmlSettingsManager::Instance ()->
		property ("UpdateInterval").toInt ();
	Q_FOREACH (Filter f, Filters_)
		if (f.SD_.LastDateTime_.daysTo (current) > days)
			if (Load (f.SD_.URL_, f.SD_.Name_))
				Remove (f.SD_.Filename_);
}

bool Core::Load (const QUrl& url, const QString& subscrName)
{
	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");

	QString name = QFileInfo (url.path ()).fileName ();
	QString path = home.absoluteFilePath (name);

	LeechCraft::DownloadEntity e =
		LeechCraft::Util::MakeEntity (url.toString ().toUtf8 (),
			path,
			LeechCraft::Internal |
				LeechCraft::DoNotNotifyUser |
				LeechCraft::DoNotSaveInHistory |
				LeechCraft::NotPersistent |
				LeechCraft::DoNotAnnounceEntity);

	int id = -1;
	QObject *pr;
	emit delegateEntity (e, &id, &pr);
	if (id == -1)
	{
		QMessageBox::critical (0,
				tr ("Error"),
				tr ("The subscription wasn't delegated."));
		qWarning () << Q_FUNC_INFO
			<< url.toString ().toUtf8 ();
		return false;
	}

	HandleProvider (pr);
	PendingJob pj =
	{
		path,
		name,
		subscrName,
		url
	};
	PendingJobs_ [id] = pj;
	return true;
}

void Core::Remove (const QString& fileName)
{
	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");
	home.remove (fileName);

	QList<Filter>::iterator pos = std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FilterFinderBase::TFilename_> (fileName));
	if (pos != Filters_.end ())
	{
		int row = std::distance (Filters_.begin (), pos);
		beginRemoveRows (QModelIndex (), row, row);
		Filters_.erase (pos);
		endRemoveRows ();
		WriteSettings ();
	}
	else
		qWarning () << Q_FUNC_INFO
			<< "could not find filter for name"
			<< fileName;
}

void Core::WriteSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CleanWeb");
	settings.beginWriteArray ("Subscriptions");
	settings.remove ("");

	int i = 0;
	Q_FOREACH (Filter f, Filters_)
	{
		settings.setArrayIndex (i++);
		settings.setValue ("URL", f.SD_.URL_);
		settings.setValue ("name", f.SD_.Name_);
		settings.setValue ("fileName", f.SD_.Filename_);
		settings.setValue ("lastDateTime", f.SD_.LastDateTime_);
	}

	settings.endArray ();
}

void Core::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CleanWeb");
	int size = settings.beginReadArray ("Subscriptions");

	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		SubscriptionData sd =
		{
			settings.value ("URL").toUrl (),
			settings.value ("name").toString (),
			settings.value ("fileName").toString (),
			settings.value ("lastDateTime").toDateTime ()
		};
		if (!AssignSD (sd))
			qWarning () << Q_FUNC_INFO
				<< "could not find filter for name"
				<< sd.Filename_;
	}

	settings.endArray ();
}

bool Core::AssignSD (const SubscriptionData& sd)
{
	QList<Filter>::iterator pos =
		std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FilterFinderBase::TFilename_> (sd.Filename_));
	if (pos != Filters_.end ())
	{
		pos->SD_ = sd;
		int row = std::distance (Filters_.begin (), pos);
		emit dataChanged (index (row, 0), index (row, columnCount () - 1));
		return true;
	}
	else
		return false;
}

void Core::handleJobFinished (int id)
{
	if (!PendingJobs_.contains (id))
		return;

	PendingJob pj = PendingJobs_ [id];
	SubscriptionData sd =
	{
		pj.URL_,
		pj.Subscr_,
		pj.FileName_,
		QDateTime::currentDateTime ()
	};
	Parse (pj.FullName_);
	PendingJobs_.remove (id);
	if (!AssignSD (sd))
		qWarning () << Q_FUNC_INFO
			<< "could not find filter for name"
			<< sd.Filename_;
	WriteSettings ();
}

void Core::handleJobError (int id, IDownload::Error)
{
	if (!PendingJobs_.contains (id))
		return;

	PendingJobs_.remove (id);
}

