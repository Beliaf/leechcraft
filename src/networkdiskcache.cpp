#include "networkdiskcache.h"
#include <QtDebug>
#include <QDateTime>

NetworkDiskCache::NetworkDiskCache (QObject *parent)
: QNetworkDiskCache (parent)
{
}

QIODevice* NetworkDiskCache::prepare (const QNetworkCacheMetaData& metadata)
{
	// Some quirks for disk cache, for example, don't cache https://
	// stuff as well as scripts.
	if (QString ("4.5.1") == qVersion () &&
			(metadata.url ().scheme () == "https" ||
			 metadata.url ().path ().endsWith ("js")))
		return 0;
	else
		return QNetworkDiskCache::prepare (metadata);
}

