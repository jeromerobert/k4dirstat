/*
 *   File name:	ktreemapview.h
 *   Summary:	High level classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-05-11
 *
 *   $Id: ktreemapview.h,v 1.1 2002/05/12 15:54:59 hundhammer Exp $
 *
 */

#ifndef KTreeMapView_h
#define KTreeMapView_h

#include <qframe.h>

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif


namespace KDirStat
{
    class KTreeMapTile;
    class KDirTree;
    
    class KTreeMapView:	public QFrame
    {
	Q_OBJECT

    public:
	/**
	 * Default constructor.
	 **/
	KTreeMapView( KDirTree * tree, QWidget * parent = 0 );

	/**
	 * Destructor.
	 **/
	virtual ~KTreeMapView();


    protected:
	
	// Data members

	KTreeMapTile *	_rootTile;
    };

}	// namespace KDirStat


#endif // ifndef KTreeMapView_h


// EOF
