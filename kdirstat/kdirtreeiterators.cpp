/*
 *   File name:	kdirtreeiterators.h
 *   Summary:	Support classes for KDirStat - KDirTree iterator classes
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2001-08-08
 *
 *   $Id: kdirtreeiterators.cpp,v 1.2 2002/01/07 09:07:05 hundhammer Exp $
 *
 */


#include "kdirtreeiterators.h"

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
	    // kdDebug() << k_funcinfo << " direct child " << _current << endl;
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
		// kdDebug() << k_funcinfo << " dot entry " << _current << endl;
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
			// kdDebug() << k_funcinfo << " dot entry child " << _current << endl;
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






KFileInfoSortedIterator::KFileInfoSortedIterator( KFileInfo *		parent,
						  KDotEntryPolicy	dotEntryPolicy,
						  KFileInfoSortOrder	sortOrder,
						  bool			ascending )
    : KFileInfoIterator( parent, dotEntryPolicy, false )
{
    _sortOrder		= sortOrder;
    _ascending		= ascending;

    _childrenList = new KFileInfoList( _sortOrder, _ascending );
    CHECK_PTR( _childrenList );

    if ( _sortOrder == KSortByName )
    {
	makeDefaultOrderChildrenList();
    }
    else
    {
	KFileInfoIterator it( _parent, _policy );

	while ( *it )
	{
	    _childrenList->append( *it );
	    ++it;
	}

	_childrenList->sort();
    }

    _current = _childrenList->first();
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

	child = dotEntryChildrenList.first();

	while ( child )
	{
	    _childrenList->append( child );
	    child = dotEntryChildrenList.next();
	}
    }
}


void KFileInfoSortedIterator::next()
{
    _current = _childrenList->next();
}






KFileInfoList::KFileInfoList( KFileInfoSortOrder sortOrder, bool ascending )
    : QList<KFileInfo>()
{
    _sortOrder	= sortOrder;
    _ascending	= ascending;
}


KFileInfoList::~KFileInfoList()
{
    // NOP
}


int
KFileInfoList::compareItems( QCollection::Item it1, QCollection::Item it2 )
{
    if ( it1 == it2 )
	return 0;

    KFileInfo *file1 = (KFileInfo *) it1;
    KFileInfo *file2 = (KFileInfo *) it2;

    int result = 0;

    switch ( _sortOrder )
    {
	case KUnsorted:
	    return 1;

	case KSortByName:
	    result = QString::compare( file1->name(), file2->name() );
	    break;

	case KSortByTotalSize:
	    result = compare<KFileSize>( file1->totalSize(), file2->totalSize() );
	    break;

	case KSortByLatestMtime:
	    result = compare<time_t>( file1->latestMtime(), file2->latestMtime() );
	    break;
    }

    return _ascending ? result : -result;
}



// EOF
