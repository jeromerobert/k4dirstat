/*
 *   File name: ktreemapview.cpp
 *   Summary:	High level classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2003-01-07
 */


#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include <qevent.h>

#include "kdirtree.h"
#include "ktreemapview.h"
#include "ktreemaptile.h"


using namespace KDirStat;

#define UpdateMinSize	20



KTreemapView::KTreemapView( KDirTree * tree, QWidget * parent, const QSize & initialSize )
    : QCanvasView( parent )
    , _tree( tree )
    , _rootTile( 0 )
    , _selectedTile( 0 )
    , _selectionRect( 0 )
{
    // kdDebug() << k_funcinfo << endl;

    readConfig();

    // Default values for light sources taken from Wiik / Wetering's paper
    // about "cushion treemaps".

    _lightX		= 0.09759;
    _lightY		= 0.19518;
    _lightZ		= 0.9759;

    if ( _autoResize )
    {
	setHScrollBarMode( AlwaysOff );
	setVScrollBarMode( AlwaysOff );
    }

    if ( initialSize.isValid() )
	resize( initialSize );

    if ( tree && tree->root() )
    {
	if ( ! _rootTile )
	{
	    // The treemap might already be created indirectly by
	    // rebuildTreemap() called from resizeEvent() triggered by resize()
	    // above. If this is so, don't do it again.

	    rebuildTreemap( tree->root() );
	}
    }

    connect( this,	SIGNAL( selectionChanged( KFileInfo * ) ),
	     tree,	SLOT  ( selectItem	( KFileInfo * ) ) );

    connect( tree,	SIGNAL( selectionChanged( KFileInfo * ) ),
	     this,	SLOT  ( selectTile	( KFileInfo * ) ) );

    connect( tree,	SIGNAL( childDeleted()	 ),
	     this,	SLOT  ( rebuildTreemap() ) );
}


KTreemapView::~KTreemapView()
{
}


void
KTreemapView::readConfig()
{
    KConfig * config = kapp->config();
    config->setGroup( "Treemaps" );

    _ambientLight	= config->readNumEntry( "AmbientLight"		,  DefaultAmbientLight );
    _lightIntensity	= 255 - _ambientLight;

    _heightScaleFactor	= config->readDoubleNumEntry( "HeightScaleFactor" , DefaultHeightScaleFactor );
    _autoResize		= config->readBoolEntry( "AutoResize"		, true	);
    _squarify		= config->readBoolEntry( "Squarify"		, true	);
    _doCushionShading	= config->readBoolEntry( "CushionShading"	, true	);
    _ensureContrast	= config->readBoolEntry( "EnsureContrast"	, true	);
    _forceCushionGrid	= config->readBoolEntry( "ForceCushionGrid"	, false	);
    _minTileSize	= config->readNumEntry ( "MinTileSize"		, DefaultMinTileSize );

    _highlightColor	= readColorEntry( config, "HighlightColor"	, red			     );
    _cushionGridColor	= readColorEntry( config, "CushionGridColor"	, QColor( 0x80, 0x80, 0x80 ) );
    _outlineColor	= readColorEntry( config, "OutlineColor"	, black			     );
    _fileFillColor	= readColorEntry( config, "FileFillColor"	, QColor( 0xde, 0x8d, 0x53 ) );
    _dirFillColor	= readColorEntry( config, "DirFillColor"	, QColor( 0x10, 0x7d, 0xb4 ) );

    if ( _autoResize )
    {
	setHScrollBarMode( AlwaysOff );
	setVScrollBarMode( AlwaysOff );
    }
    else
    {
	setHScrollBarMode( QScrollView::Auto );
	setVScrollBarMode( QScrollView::Auto );
    }
}


QColor
KTreemapView::readColorEntry( KConfig * config, const char * entryName, QColor defaultColor )
{
    return config->readColorEntry( entryName, &defaultColor );
}


KTreemapTile *
KTreemapView::tileAt( QPoint pos )
{
    KTreemapTile * tile = 0;

    QCanvasItemList coll = canvas()->collisions( pos );
    QCanvasItemList::Iterator it = coll.begin();

    while ( it != coll.end() && tile == 0 )
    {
	tile = dynamic_cast<KTreemapTile *> (*it);
	++it;
    }

    return tile;
}


void
KTreemapView::contentsMousePressEvent( QMouseEvent * event )
{
    // kdDebug() << k_funcinfo << endl;

    KTreemapTile * tile = tileAt( event->pos() );

    switch ( event->button() )
    {
	case LeftButton:
	    selectTile( tile );
	    break;

	case MidButton:
	    // Select clicked tile's parent, if available

	    if ( _selectedTile &&
		 _selectedTile->rect().contains( event->pos() ) )
	    {
		if ( _selectedTile->parentTile() )
		    tile = _selectedTile->parentTile();
	    }

	    // Intentionally handling the middle button like the left button if
	    // the user clicked outside the (old) selected tile: Simply select
	    // the clicked tile. This makes using this middle mouse button
	    // intuitive: It can be used very much like the left mouse button,
	    // but it has added functionality. Plus, it cycles back to the
	    // clicked tile if the user has already clicked all the way up the
	    // hierarchy (i.e. the topmost directory is highlighted).

	    selectTile( tile );
	    break;

	case RightButton:
	    if ( tile )
	    {
		selectTile( tile );
		emit contextMenu( tile, event->pos() );
	    }
	    break;
    }
}


void
KTreemapView::contentsMouseDoubleClickEvent( QMouseEvent * event )
{
    // kdDebug() << k_funcinfo << endl;

    KTreemapTile * tile = tileAt( event->pos() );

    switch ( event->button() )
    {
	case LeftButton:
	    if ( tile )
	    {
		selectTile( tile );
		zoomIn();
	    }
	    break;

	case MidButton:
	    if ( _rootTile )
		rebuildTreemap( _rootTile->orig() );
	    else if ( _tree && _tree->root() )
		rebuildTreemap( _tree->root() );
	    break;

	case RightButton:
	    zoomOut();
	    break;
    }
}


void
KTreemapView::zoomIn()
{
    if ( ! _selectedTile || ! _rootTile )
	return;

    KTreemapTile * newRootTile = _selectedTile;

    while ( newRootTile->parentTile() != _rootTile &&
	    newRootTile->parentTile() ) // This should never happen, but who knows?
    {
	newRootTile = newRootTile->parentTile();
    }

    if ( newRootTile )
    {
	KFileInfo * newRoot = newRootTile->orig();

	if ( newRoot->isDir() || newRoot->isDotEntry() )
	    rebuildTreemap( newRoot );
    }
}


void
KTreemapView::zoomOut()
{
    if ( _rootTile )
    {
	KFileInfo * root = _rootTile->orig();

	if ( root->parent() )
	    root = root->parent();

	rebuildTreemap( root );
    }
}


void
KTreemapView::selectParent()
{
    if ( _selectedTile && _selectedTile->parentTile() )
	selectTile( _selectedTile->parentTile() );
}


bool
KTreemapView::canZoomIn() const
{
    if ( ! _selectedTile || ! _rootTile )
	return false;

    KTreemapTile * newRootTile = _selectedTile;

    while ( newRootTile->parentTile() != _rootTile &&
	    newRootTile->parentTile() ) // This should never happen, but who knows?
    {
	newRootTile = newRootTile->parentTile();
    }

    if ( newRootTile )
    {
	KFileInfo * newRoot = newRootTile->orig();

	if ( newRoot->isDir() || newRoot->isDotEntry() )
	    return true;
    }

    return false;
}


bool
KTreemapView::canZoomOut() const
{
    if ( ! _rootTile || ! _tree->root() )
	return false;

    return _rootTile->orig() != _tree->root();
}


bool
KTreemapView::canSelectParent() const
{
    return _selectedTile && _selectedTile->parentTile();
}


void
KTreemapView::rebuildTreemap()
{
    if ( ! canvas() )
    {
	kdError() << k_funcinfo << "No canvas created yet!" << endl;
	return;
    }

    _selectedTile = 0;

    rebuildTreemap( _rootTile ? _rootTile->orig() : _tree->root(),
		    canvas()->size() );
}


void
KTreemapView::rebuildTreemap( KFileInfo *	newRoot,
			      const QSize &	newSz )
{
    // kdDebug() << k_funcinfo << endl;

    QSize newSize = newSz;

    if ( newSz.isEmpty() )
	newSize = visibleSize();

    // Save current state
    KFileInfo * savedSelection = _selectedTile ? _selectedTile->orig() : 0;


    // Delete all old stuff. Unfortunately, there is no QCanvas::clear(), so we
    // have to delete the entire canvas.

    if ( canvas() )
	delete canvas();

    _selectedTile	= 0;
    _selectionRect	= 0;
    _rootTile		= 0;


    // Re-create a new canvas

    QCanvas * canv = new QCanvas( this );
    canv->resize( newSize.width(), newSize.height() );
    setCanvas( canv );

    if ( newSize.width() >= UpdateMinSize && newSize.height() >= UpdateMinSize )
    {
	// The treemap contents is displayed if larger than a certain minimum
	// visible size. This is an easy way for the user to avoid
	// time-consuming delays when deleting a lot of files: Simply make the
	// treemap (sub-) window very small.

	// Fill the new canvas

	if ( newRoot )
	{
	    _rootTile = new KTreemapTile( this,		// parentView
					  0,		// parentTile
					  newRoot,	// orig
					  QRect( QPoint( 0, 0), newSize ),
					  KTreemapAuto );
	}


	// Restore the old state

	if ( savedSelection )
	    selectTile( savedSelection );
    }
    else
    {
	// kdDebug() << "Too small - suppressing treemap contents" << endl;
    }

    emit treemapChanged();
}


void
KTreemapView::resizeEvent( QResizeEvent * event )
{
    QCanvasView::resizeEvent( event );

    if ( _autoResize )
    {
	bool tooSmall =
	    event->size().width()  < UpdateMinSize ||
	    event->size().height() < UpdateMinSize;

	if ( tooSmall && _rootTile )
	{
	    // kdDebug() << "Suppressing treemap contents" << endl;
	    rebuildTreemap( _rootTile->orig() );
	}
	else if ( ! tooSmall && ! _rootTile )
	{
	    if ( _tree->root() )
	    {
		// kdDebug() << "Redisplaying suppressed treemap contents" << endl;
		rebuildTreemap( _tree->root() );
	    }
	}
	else if ( _rootTile )
	{
	    // kdDebug() << "Auto-resizing treemap" << endl;
	    rebuildTreemap( _rootTile->orig() );
	}
    }
}


void
KTreemapView::selectTile( KTreemapTile * tile )
{
    // kdDebug() << k_funcinfo << endl;

    KTreemapTile * oldSelection = _selectedTile;
    _selectedTile = tile;


    // Handle selection (highlight) rectangle

    if ( _selectedTile )
    {
	if ( ! _selectionRect )
	    _selectionRect = new KTreemapSelectionRect( canvas(), _highlightColor );
    }

    if ( _selectionRect )
	_selectionRect->highlight( _selectedTile );

    canvas()->update();

    if ( oldSelection != _selectedTile )
    {
	emit selectionChanged( _selectedTile ? _selectedTile->orig() : 0 );
    }
}


void
KTreemapView::selectTile( KFileInfo * node )
{
    selectTile( findTile( node ) );
}



KTreemapTile *
KTreemapView::findTile( KFileInfo * node )
{
    if ( ! node )
	return 0;

    QCanvasItemList itemList = canvas()->allItems();
    QCanvasItemList::Iterator it = itemList.begin();

    while ( it != itemList.end() )
    {
	KTreemapTile * tile = dynamic_cast<KTreemapTile *> (*it);

	if ( tile && tile->orig() == node )
	    return tile;

	++it;
    }

    return 0;
}


QSize
KTreemapView::visibleSize()
{
    ScrollBarMode oldHMode = hScrollBarMode();
    ScrollBarMode oldVMode = vScrollBarMode();

    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOff );

    QSize size = QSize( QCanvasView::visibleWidth(),
			QCanvasView::visibleHeight() );

    setHScrollBarMode( oldHMode );
    setVScrollBarMode( oldVMode );

    return size;
}






KTreemapSelectionRect::KTreemapSelectionRect( QCanvas * canvas, const QColor & color )
    : QCanvasRectangle( canvas )
{
    setPen( QPen( color, 2 ) );
    setZ( 1e10 );		// Higher than everything else
}



void
KTreemapSelectionRect::highlight( KTreemapTile * tile )
{
    if ( tile )
    {
	QRect tileRect = tile->rect();

	move( tileRect.x(), tileRect.y() );
	setSize( tileRect.width(), tileRect.height() );

	if ( ! isVisible() )
	    show();
    }
    else
    {
	if ( isVisible() )
	    hide();
    }
}



// EOF
