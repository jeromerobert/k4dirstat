/*
 *   File name:	ktreemaptile.cpp
 *   Summary:	High level classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-05-11
 *
 *   $Id: ktreemaptile.cpp,v 1.1 2002/05/12 15:53:51 hundhammer Exp $
 *
 */

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>

#include "ktreemaptile.h"
#include "kdirtreeiterators.h"
#include "kdirtreeview.h"


using namespace KDirStat;

#define MinTileSize	3


KTreeMapTile::KTreeMapTile( QWidget * 		parent,
			    KFileInfo *		orig,
			    QRect		rect,
			    Orientation		orientation )
    : QFrame( parent )
    , _orig( orig )
{
    if ( orig->totalSize() == 0 )
	return;

    setGeometry( rect );
    Orientation dir = orientation;
    Orientation childDir = orientation;

#if 0
    kdDebug() << "Creating treemap tile for " << orig
	      << endl
	      << "   inside " 
	      << rect.x() << ", " << rect.y()
	      << " w: " << rect.width()
	      << " h: " << rect.height()
	      << " or: " << ( dir == Hor ? "Hor" : ( dir == Vert ? "Vert" : "Auto" ) )
	      << endl;
#endif

    if ( dir == Auto )
	dir = rect.width() > rect.height() ? Hor : Vert;

    if ( orientation == Hor )	childDir = Vert;
    if ( orientation == Vert )	childDir = Hor;
    
    int offset	= dir == Hor ? rect.x() : rect.y();
    int size	= dir == Hor ? rect.width() : rect.height();
    int count 	= 0;

    double scale = (double) size / (double) _orig->totalSize();

    KFileInfoIterator it( orig );

    while ( *it )
    {
	int childSize = 0;
	
	childSize = (int) ( scale * (*it)->totalSize() );

#if 0
	kdDebug() << "Making tile for child " << (*it)
		  << endl
		  << "   parent size: " << formatSize( _orig->totalSize() )
		  << " child size: " << formatSize( (*it)->totalSize() )
		  << " (" << ( 100.0 * (*it)->totalSize() ) / _orig->totalSize() << "%)"
		  << " -> " << childSize << " of " << size
		  << endl;
#endif

	if ( childSize >= MinTileSize )
	{
#if 0
	    kdDebug() << "Subdividing " << (*it) << " size " << childSize << endl;
#endif
	    QRect childRect;

	    if ( dir == Hor )
		childRect = QRect( offset, 0, childSize, rect.height() );
	    else
		childRect = QRect( 0, offset, rect.width(), childSize );

	    new KTreeMapTile( this, *it, childRect, childDir );
	    
	    offset += childSize;
	}
	    
	++count;
	++it;
    }

    if ( count == 0 )	// No children at all - this is a leaf
    {
	setLineWidth( 2 );
	setMidLineWidth( 2 );
	setFrameStyle( QFrame::Panel | QFrame::Raised );

#if 0
	kdDebug() << "Leaf " << orig
		  << " at " << rect.x()
		  << ", " << rect.y() 
		  << " w: " << rect.width()
		  << " h: " << rect.height()
		  << endl;
#endif
    }

    connect( this,          SIGNAL( selectionChanged( KFileInfo * ) ),
	     _orig->tree(), SLOT  ( selectItem      ( KFileInfo * ) ) );

#if 0
    // TO DO
    // TO DO
    // TO DO
    connect( _orig->tree(), SIGNAL( selectionChanged( KFileInfo * ) ),
	     this,          SLOT  ( selectItem      ( KFileInfo * ) ) );
    // TO DO
    // TO DO
    // TO DO
#endif
}


KTreeMapTile::~KTreeMapTile()
{
}


void
KTreeMapTile::mouseReleaseEvent( QMouseEvent * )
{
    // kdDebug() << _orig->debugUrl() << endl;
    kdDebug() << "Selected " << _orig << endl;
    emit selectionChanged( _orig );
}





// EOF
