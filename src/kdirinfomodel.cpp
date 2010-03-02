#include <klocale.h>

#include "kdirinfomodel.h"
#include "kdirinfo.h"

using namespace KDirStat;

KDirInfoModel::KDirInfoModel(KFileInfo *info, QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = info;
}

int KDirInfoModel::columnCount(const QModelIndex &parent) const{
    return 9;
}

int KDirInfoModel::rowCount(const QModelIndex &parent) const{
    if (parent.column() > 0)
	return 0;

    KFileInfo *parentItem;

    if (!parent.isValid())
	parentItem = rootItem;
    else
	parentItem = static_cast<KFileInfo*>(parent.internalPointer());

    return parentItem->totalItems();
}

QVariant KDirInfoModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
	return QVariant();

    if (role != Qt::DisplayRole)
	return QVariant();

    KFileInfo* fileInfo = static_cast<KFileInfo*>(index.internalPointer());

    switch(index.column()) {
    case 0:
	return fileInfo->name();
    case 1:
	return fileInfo->pendingReadJobs();
    case 2:
	if ( fileInfo == rootItem ) return QVariant();
	return  (float)fileInfo->totalSize()/fileInfo->parent()->totalSize();
    case 3:
	return fileInfo->totalSize();
    case 4:
	return fileInfo->size();
    case 5:
	return fileInfo->totalItems();
    case 6:
	return fileInfo->totalFiles();
    case 7:
	return fileInfo->totalSubDirs();
    case 8:
	return int(fileInfo->latestMtime());
    default:
	return QVariant();
    }
}

QModelIndex KDirInfoModel::index(int row, int column, const QModelIndex &parent) const{
    if (!hasIndex(row, column, parent))
	 return QModelIndex();

    KFileInfo *parentItem;

    if (!parent.isValid())
	parentItem = rootItem;
    else
	parentItem = static_cast<KFileInfo*>(parent.internalPointer());
    KFileInfo *childItem = parentItem->firstChild();
    // FIXME change this loop if row is 1-indexed
    while( childItem && childItem->row() != row){
    //for (int i = 0; i< row; i++){
//	if(childItem)
	childItem = childItem->next();
    }
    if (childItem)
	return createIndex(row, column, childItem);
    else
	return QModelIndex();
}

QModelIndex KDirInfoModel::parent(const QModelIndex &child) const{
    if (!child.isValid())
	return QModelIndex();

    KFileInfo *childItem = static_cast<KFileInfo*>(child.internalPointer());
    KFileInfo *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
	return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant KDirInfoModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
	switch (section) {
	case 0:
	    return i18n( "Name"			);
	case 1:
	    return i18n( "Subtree Percentage" 	);
	case 2:
	    return i18n( "Percentage"		);
	case 3:
	    return i18n( "Subtree Total"	);
	case 4:
	    return i18n( "Own Size"		);
	case 5:
	    return i18n( "Items"		);
	case 6:
	    return i18n( "Files"		);
	case 7:
	    return i18n( "Subdirs"		);
	case 8:
	    return i18n( "Last Change"		);
	default:
	    return QVariant();
	}
    }

    return QVariant();
}

Qt::ItemFlags KDirInfoModel::flags(const QModelIndex &index) const{
    if (!index.isValid())
	return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
