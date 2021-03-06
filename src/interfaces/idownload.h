#ifndef INTERFACES_IDOWNLOAD_H
#define INTERFACES_IDOWNLOAD_H
#include <QByteArray>
#include <QtPlugin>
#include "structures.h"

/** @brief Common interface for all the downloaders.
 *
 * Plugins which provide downloading capabilities and want to be visible
 * by LeechCraft and other plugins as download providers should
 * implement this interface.
 *
 * @sa IJobHolder, IEntityHandler
 */
class IDownload
{
public:
	enum Error
	{
		EUnknown
		, ENoError
		, ENotFound
		, EAccessDenied
		, ELocalError
	};
    typedef unsigned long int JobID_t;

	/** @brief Returns download speed.
	 *
	 * Returns summed up download speed of the plugin. The value is
	 * primarily used in the interface as there are no ways of
	 * controlling of bandwidth's usage of a particular plugin.
	 *
	 * @return Download speed in bytes.
	 *
	 * @sa GetUploadSpeed
	 */
    virtual qint64 GetDownloadSpeed () const = 0;
	/** @brief Returns upload speed.
	 *
	 * Returns summed up upload speed of the plugin. The value is
	 * primarily used in the interface as there are no ways of
	 * controlling of bandwidth's usage of a particular plugin.
	 *
	 * @return Upload speed in bytes.
	 *
	 * @sa GetDownloadSpeed
	 */
    virtual qint64 GetUploadSpeed () const = 0;

	/** @brief Starts all tasks.
	 *
	 * This is called by LeechCraft when it wants all plugins to start
	 * all of its tasks.
	 */
    virtual void StartAll () = 0;
	/** @brief Stops all tasks.
	 *
	 * This is called by LeechCraft when it wants all plugins to stop
	 * all of its tasks.
	 */
    virtual void StopAll () = 0;

	/** @brief Returns whether plugin can handle given entity.
	 *
	 * This function is used to query every loaded plugin providing the
	 * IDownload interface whether it could handle the entity entered by
	 * user or generated automatically with given task parameters.
	 * Entity could be anything from file name to URL to all kinds of
	 * hashes like Magnet links.
	 *
	 * @param[in] entity A DownloadEntity structure.
	 *
	 * @sa AddJob
	 * @sa LeechCraft::DownloadEntity
	 */
    virtual bool CouldDownload (const LeechCraft::DownloadEntity& entity) const = 0;

	/** @brief Adds the job with given parameters.
	 *
	 * Adds the job to the downloader and returns the ID of the newly
	 * added job back to identify it.
	 *
	 * @param[in] entity A DownloadEntity structure.
	 * @return ID of the job for the other plugins to use.
	 *
	 * @sa LeechCraft::DownloadEntity
	 */
    virtual int AddJob (LeechCraft::DownloadEntity entity) = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IDownload () {}
};

Q_DECLARE_INTERFACE (IDownload, "org.Deviant.LeechCraft.IDownload/1.0");

#endif

