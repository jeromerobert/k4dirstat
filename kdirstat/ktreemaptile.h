/*
 *   File name:	ktreemaptile.h
 *   Summary:	High level classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-05-11
 *
 *   $Id: ktreemaptile.h,v 1.1 2002/05/12 15:53:51 hundhammer Exp $
 *
 */

#ifndef KTreeMapTile_h
#define KTreeMapTile_h



#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif


#include <qframe.h>
#include <qrect.h>
#include "kdirtree.h"


// Forward declarations
class QWidget;
class QFrame;


namespace KDirStat
{
    /**
     * This is the basic building block of a treemap view: One single tile of a
     * treemap. If it corresponds to a leaf in the tree, it will be visible as
     * one tile (one rectangle) of the treemap. If it has children, it will be
     * subdivided again.
     *
     * @short Basic building block of a treemap 
     **/
    class KTreeMapTile:	public QFrame
    {
	Q_OBJECT

    public:
	
	enum Orientation { Hor, Vert, Auto };
	
	/**
	 * Constructor: Create a treemap tile from 'fileinfo' that fits into a
	 * rectangle 'rect' inside 'parent'.
	 *
	 * 'orientation' is the direction for further subdivision. 'Auto'
	 * selects the wider direction inside 'rect'.
	 **/
	KTreeMapTile( QWidget * 	parent,
		      KFileInfo *	orig,
		      QRect		rect,
		      Orientation	orientation = Auto );

	/**
	 * Destructor.
	 **/
	virtual ~KTreeMapTile();


	/**
	 * Returns the original @ref KFileInfo item that corresponds to this
	 * treemap tile. 
	 **/
	KFileInfo *	orig() { return _orig; }


    signals:

	/**
	 * Emitted when the currently selected item changes.
	 * Caution: 'item' may be 0 when the selection is cleared.
	 **/
	void selectionChanged( KFileInfo *item );


    protected:

	/**
	 * Reimplemented so a treemap tile can react to mouse clicks.
	 **/
	virtual void mouseReleaseEvent( QMouseEvent * );

	
	KFileInfo *	_orig;
    };

}	// namespace KDirStat


#endif // ifndef KTreeMapTile_h


// EOF
