#ifndef PLUGININTERFACE_MERGEMODEL_H
#define PLUGININTERFACE_MERGEMODEL_H
#include <deque>
#include <QAbstractProxyModel>
#include <QStringList>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		/** Merges data from multiple source models into one resulting
		 * model and provides means to lookup models by row, get
		 * starting rows for a model etc.
		 *
		 * To add a new source model, one should use AddModel() as
		 * setSourceModel() throws an std::runtime_error exception.
		 *
		 * Currently it doesn't support hierarchical source models.
		 * Seems like it would never support it at least someone would
		 * try to implement it.
		 */
		class PLUGININTERFACE_API MergeModel : public QAbstractProxyModel
		{
			Q_OBJECT
		protected:
			typedef std::deque<QAbstractItemModel*> models_t;
			models_t Models_;
		private:
			QStringList Headers_;
		public:
			typedef models_t::iterator iterator;
			typedef models_t::const_iterator const_iterator;

			MergeModel (const QStringList&, QObject* = 0);
			virtual ~MergeModel ();

			virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
			virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
			virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
			virtual Qt::ItemFlags flags (const QModelIndex&) const;
			virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			virtual QModelIndex parent (const QModelIndex&) const;
			virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

			/** Returns the model index in the MergeModel given the
			 * index from the source model.
			 *
			 * @param[in] index Source index.
			 * @return MergeModel's index.
			 */
			virtual QModelIndex mapFromSource (const QModelIndex& index) const;

			/** Returns the source model index corresponding to the
			 * given index from the sorting filter model.
			 *
			 * @param[in] index MergeModel's index.
			 * @return Source index.
			 */
			virtual QModelIndex mapToSource (const QModelIndex& index) const;

			/** You shouldn't use this function because its semantics in
			 * the context of multiple source models aren't clearly
			 * defined. Calling this function results in
			 * std::runtime_error.
			 *
			 * @exception std::runtime_error No matter what, you'd get
			 * it.
			 */
			virtual void setSourceModel (QAbstractItemModel*);

			/** Adds a model to the list of source models. The newly
			 * added model is appended to the end.
			 *
			 * If the model already exists in the list, it is added
			 * again, and bad things would happen, as all the signals and
			 * slots would be connected and called twice. So it's your
			 * duty to ensure that you don't add the same model more than
			 * once.
			 *
			 * @param[in] model The model to append to the list.
			 */
			void AddModel (QAbstractItemModel *model);

			/** Removes a model from the list of source models. If there
			 * is no such model, this function does nothing.
			 *
			 * @param[in] model The model to remove from the list.
			 */
			void RemoveModel (QAbstractItemModel *model);

			/** Returns the number of child models in the merger.
			 *
			 * @return The number of child models.
			 */
			size_t Size () const;

			/** Returns a const_iterator corresponding to the passed
			 * model, or one-past-end if no such model is found.
			 *
			 * @param[in] model The model to find.
			 * @return The iterator.
			 */
			const_iterator FindModel (const QAbstractItemModel *model) const;

			/** This is an overloaded function provided for convenience.
			 * Non-const and returns a non-const iterator.
			 *
			 * @param[in] model The model to find.
			 * @return The iterator.
			 */
			iterator FindModel (const QAbstractItemModel *model);
			
			/** Returns the row in the resulting MergeModel from which do
			 * begin rows which belong to the model corresponding to the
			 * given const_iterator.
			 *
			 * @param[in] iterator The iterator corresponding to the
			 * model.
			 * @return The starting row.
			 */
			int GetStartingRow (const_iterator iterator) const;
			
			/** Returns the model that corresponds to the given row. If
			 * there is no such model, throws.
			 *
			 * @param[in] row The row that should be identified.
			 * @return Iterator associated with the model.
			 *
			 * @exception std::runtime_error Throws if there is no model
			 * for such row.
			 */
			const_iterator GetModelForRow (int row) const;

			/** This is an overloaded function provided for convenience.
			 * Non-const and returns a non-const iterator.
			 *
			 * @param[in] row The row that should be identified.
			 * @return Iterator associated with the model.
			 *
			 * @exception std::runtime_error Throws if there is no model
			 * for such row.
			 */
			iterator GetModelForRow (int row);
		public Q_SLOTS:
			void handleColumnsAboutToBeInserted (const QModelIndex&, int, int);
			void handleColumnsAboutToBeRemoved (const QModelIndex&, int, int);
			void handleColumnsInserted (const QModelIndex&, int, int);
			void handleColumnsRemoved (const QModelIndex&, int, int);
			void handleDataChanged (const QModelIndex&, const QModelIndex&);
			void handleRowsAboutToBeInserted (const QModelIndex&, int, int);
			void handleRowsAboutToBeRemoved (const QModelIndex&, int, int);
			void handleRowsInserted (const QModelIndex&, int, int);
			void handleRowsRemoved (const QModelIndex&, int, int);
		protected:
			/** This virtual function could be overridden to provide
			 * custom filtering facilities. If the row in the model
			 * should be merged into the resulting model, this function
			 * should return true, otherwise if it returns false the row
			 * would be filtered out.
			 */
			virtual bool AcceptsRow (QAbstractItemModel *model, int row) const;
		private:
			int RowCount (QAbstractItemModel*) const;
		};
	};
};

#endif

