/*
 *   File name:	kdirtreeiterators.h
 *   Summary:	Support classes for KDirStat - KDirTree iterator classes
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 *
 *   Updated:	2010-02-01
 */


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include "kdirtreeiterators.h"
#include "kdirtree.h"


using namespace KDirStat;


KFileInfoIterator::KFileInfoIterator( KFileInfo *	parent,
				      KDotEntryPolicy	dotEntryPolicy )
{
    init( parent,
	  dotEntryPolicy,
	  true );		// callNext
}


KFileInfoIterator::KFileInfoIterator( KFileInfo *	parent,
				      KDotEntryPolicy	dotEntryPolicy,
				      bool		callNext )
{
    init( parent, dotEntryPolicy, callNext );
}


void
KFileInfoIterator::init( KFileInfo *		parent,
			 KDotEntryPolicy	dotEntryPolicy,
			 bool			callNext )
{
    _parent	= parent;
    _policy	= dotEntryPolicy;
    _current	= 0;

    _directChildrenProcessed	= false;
    _dotEntryProcessed		= false;
    _dotEntryChildrenProcessed	= false;

    if ( callNext )
	next();
}


KFileInfoIterator::~KFileInfoIterator()
{
    // NOP
}


void KFileInfoIterator::next()
{
    if ( ! _directChildrenProcessed )
    {
	// Process direct children

	_current = _current ? _current->next() : _parent->firstChild();

	if ( ! _current )
	{
	    _directChildrenProcessed = true;
	    next();
	}
	else
	{
	    // qDebug() << k_funcinfo << " direct child " << _current << endl;
	}
    }
    else	// _directChildrenProcessed
    {
	if ( ! _dotEntryProcessed )
	{
	    // Process dot entry

	    _current = _policy == KDotEntryAsSubDir ? _parent->dotEntry() : 0;
	    _dotEntryProcessed = true;

	    if ( ! _current )
	    {
		next();
	    }
	    else
	    {
		// qDebug() << k_funcinfo << " dot entry " << _current << endl;
	    }
	}
	else	// Dot entry already processed or processing it not desired
	{
	    if ( ! _dotEntryChildrenProcessed )
	    {
		if ( _policy == KDotEntryTransparent )
		{
		    // Process dot entry children

		    _current = _current ?
			_current->next() :
			( _parent->dotEntry() ? _parent->dotEntry()->firstChild() : 0 );

		    if ( ! _current )
		    {
			_dotEntryChildrenProcessed = true;
		    }
		    else
		    {
			// qDebug() << k_funcinfo << " dot entry child " << _current << endl;
		    }
		}
		else	// _policy != KDotEntryTransparent
		{
		    _current = 0;
		    _dotEntryChildrenProcessed = true;
		}
	    }
	}
    }
}


int
KFileInfoIterator::count()
{
    int cnt = 0;

    // Count direct children

    KFileInfo *child = _parent->firstChild();

    while ( child )
    {
	cnt++;
	child = child->next();
    }


    // Handle the dot entry

    switch ( _policy )
    {
	case KDotEntryTransparent:	// Count the dot entry's children as well.
	    if ( _parent->dotEntry() )
	    {
		child = _parent->dotEntry()->firstChild();

		while ( child )
		{
		    cnt++;
		    child = child->next();
		}
	    }
	    break;

	case KDotEntryAsSubDir:		// The dot entry counts as one item.
	    if ( _parent->dotEntry() )
		cnt++;
	    break;

	case KDotEntryIgnore:		// We're done.
	    break;
    }

    return cnt;
}






KFileInfoSortedIterator::KFileInfoSortedIterator( KFileInfo *		parent,
						  KDotEntryPolicy	dotEntryPolicy,
						  KFileInfoSortOrder	sortOrder,
						  bool			ascending )
    : KFileInfoIterator( parent, dotEntryPolicy, false )
{
    _sortOrder			= sortOrder;
    _ascending			= ascending;
    _initComplete		= false;
    _childrenList		= 0;
    _current			= 0;
}


void
KFileInfoSortedIterator::delayedInit()
{
    _childrenList = new KFileInfoList( _sortOrder, _ascending );
    Q_CHECK_PTR( _childrenList );

    if ( _sortOrder == KSortByName )
    {
	makeDefaultOrderChildrenList();
    }
    else
    {
	makeChildrenList();
    }

    _current = 0;
    _initComplete = true;
}


KFileInfoSortedIterator::~KFileInfoSortedIterator()
{
    if ( _childrenList )
	delete _childrenList;
}


void KFileInfoSortedIterator::makeDefaultOrderChildrenList()
{
    // Fill children list with direct children

    KFileInfo *child = _parent->firstChild();

    while ( child )
    {
	_childrenList->append( child );
	child = child->next();
    }

    _childrenList->sort();

    if ( _policy == KDotEntryAsSubDir && _parent->dotEntry() )
    {
	// Append dot entry to the children list

	_childrenList->append( _parent->dotEntry() );
    }


    // Append the dot entry's children to the children list

    if ( _policy == KDotEntryTransparent && _parent->dotEntry() )
    {
	// Create a temporary list for the dot entry children

	KFileInfoList dotEntryChildrenList( _sortOrder, _ascending );
	child = _parent->dotEntry()->firstChild();

	while ( child )
	{
	    dotEntryChildrenList.append( child );
	    child = child->next();
	}

	dotEntryChildrenList.sort();


	// Now append all of this dot entry children list to the children list
	_childrenList->append(dotEntryChildrenList);
    }
}


void
KFileInfoSortedIterator::makeChildrenList()
{
    KFileInfoIterator it( _parent, _policy );

    while ( *it )
    {
	_childrenList->append( *it );
	++it;
    }

    _childrenList->sort();
}


KFileInfo *
KFileInfoSortedIterator::current()
{
    if ( ! _initComplete )
	delayedInit();
    if(_current < _childrenList->size())
        return _childrenList->at(_current);
    else
        return NULL;
}


void KFileInfoSortedIterator::next()
{
    if ( ! _initComplete )
	delayedInit();
    _current++;
}


bool
KFileInfoSortedIterator::finished()
{
    if ( ! _initComplete )
	delayedInit();

    return _current >= _childrenList->size();
}






KFileInfoSortedBySizeIterator::KFileInfoSortedBySizeIterator( KFileInfo *		parent,
							      KFileSize			minSize,
							      KDotEntryPolicy		dotEntryPolicy,
							      bool			ascending )
    : KFileInfoSortedIterator( parent, dotEntryPolicy, KSortByTotalSize, ascending )
    , _minSize( minSize )
{
}


void
KFileInfoSortedBySizeIterator::makeChildrenList()
{
    KFileInfoIterator it( _parent, _policy );

    while ( *it )
    {
	if ( (*it)->totalSize() >= _minSize )
	    _childrenList->append( *it );

	++it;
    }

    _childrenList->sort();
}






KFileInfoList::KFileInfoList( KFileInfoSortOrder sortOrder, bool ascending )
    : QList<KFileInfo*>()
{
    _sortOrder	= sortOrder;
    _ascending	= ascending;
}


KFileInfoList::~KFileInfoList()
{
    // NOP
}



KFileSize
KFileInfoList::sumTotalSizes()
{
    KFileSize sum = 0;
    foreach(KFileInfo * it, *this)
	sum += it->totalSize();

    return sum;
}

class KFileInfoComparator {
public:
    KFileInfoComparator(KFileInfoSortOrder sortOrder, bool ascending):
    _sortOrder(sortOrder), _ascending(ascending) {}

    bool operator()(const KFileInfo *file1, const KFileInfo * file2) const
    {
        if ( file1 == file2 )
            return false;

        int result = 0;
        KFileInfo * mf1 = const_cast<KFileInfo*>(file1);
        KFileInfo * mf2 = const_cast<KFileInfo*>(file2);
        switch ( _sortOrder )
        {
        case KSortByName:
            result = QString::compare( file1->name(), file2->name() );
            break;

        case KSortByTotalSize:
            result = compare<KFileSize>(mf1->totalSize(), mf2->totalSize() );
            break;

        case KSortByLatestMtime:
            result = compare<time_t>(mf1->latestMtime(), mf2->latestMtime() );
            break;
        default:
            Q_ASSERT(false);
        }

        return _ascending ? result < 0 : result > 0;
    }

private:
    KFileInfoSortOrder _sortOrder;
    bool _ascending;
};

void KFileInfoList::sort() {
    if(_sortOrder != KUnsorted) {
        KFileInfoComparator c(_sortOrder, _ascending);
        qSort(begin(), end(), c);
    }
}

// EOF
