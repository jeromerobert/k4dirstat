#ifndef KDIRINFOMODEL_H
#define KDIRINFOMODEL_H

#include <QAbstractItemModel>
#include "kfileinfo.h"

namespace KDirStat
{

    class KDirInfoModel : public QAbstractItemModel
    {
    public:
	KDirInfoModel(KFileInfo *info, QObject *parent=0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
			    int role = Qt::DisplayRole) const;
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const ;
	QModelIndex parent ( const QModelIndex & child ) const;
	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

    private:
	KFileInfo* rootItem;
    };

}
#endif // KDIRINFOMODEL_H
