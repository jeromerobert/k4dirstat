/*
 *   File name:	ktreemapview.cpp
 *   Summary:	High level classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-05-13
 *
 *   $Id: ktreemapview.cpp,v 1.2 2002/05/13 11:46:19 hundhammer Exp $
 *
 */

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <qlabel.h>

#include "ktreemapview.h"
#include "ktreemaptile.h"
#include "kdirtree.h"


using namespace KDirStat;


KTreeMapView::KTreeMapView( KDirTree *tree, QWidget * parent )
    : QFrame( parent )
    , _rootTile( 0 )
{
    // kdDebug() << k_funcinfo << endl;

    if ( tree && tree->root() )
    {
	QRect rect( 0, 0, 700, 250 );	// DEBUG
	_rootTile = new KTreeMapTile( this, tree->root(), rect, KTreeMapTile::Hor );
	
	resize( width(), rect.height() );
    }
}


KTreeMapView::~KTreeMapView()
{
}







// EOF
