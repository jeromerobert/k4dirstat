
/*
 *   File name:	kpacman.cpp
 *   Summary:	PacMan animation
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-01-19
 *
 *   $Id: kpacman.cpp,v 1.2 2002/01/21 10:04:52 hundhammer Exp $
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <kdebug.h>

#include "kpacman.h"


KPacManAnimation::KPacManAnimation( QWidget *	widget,
				    int		size,
				    bool	randomStart )
{
    _widget		= widget;
    _size		= size;
    _randomStart	= randomStart;
    _brush		= QBrush( Qt::yellow );
    _pos		= 0;
    _speed		= 4;
    _interval		= 100;

    _minMouth		= 10;
    _maxMouth		= 70;
    _mouthInc		= ( _maxMouth - _minMouth ) / 3;
    _mouth		= _minMouth;
    _pacManRect		= QRect( 0, 0, 0, 0 );

    restart();
}


KPacManAnimation::~KPacManAnimation()
{
}


void
KPacManAnimation::restart()
{
    _justStarted = true;

    if ( _randomStart )
    {
	_goingRight = ( rand() > ( RAND_MAX / 2 ) );

	// Initial _pos is set in animate() since the width (upon which it
	// depends) is still unknown here.
    }
    else
    {
	_goingRight	= true;
	_pos		= 0;
    }

    // Care for initial display
    _time = _time.addMSecs( _interval + 1 );
}


void
KPacManAnimation::animate( QPainter *	painter,
			   QRect 	rect )
{
    if ( _time.elapsed() < _interval )
	return;

    _time.restart();


    // Make PacMan fit into height

    int size = _size <= rect.height() ? _size : rect.height();

    if ( rect.width() < size )		// No space to animate in?
	return;				// -> forget it!


    if ( _justStarted )
    {
	_justStarted = false;

	if ( _pacManRect.width() > 0 )
	    painter->eraseRect( _pacManRect );

	if ( _randomStart )
	{
	    // Set random initial position
	    // - this depends on the width which is unknown in the constructor.

	    _pos = (int) ( (rect.width() - size ) * ( (double) rand() / RAND_MAX) );
	}
    }
    else	// ! _justStarted
    {
	// Erase last PacMan

	if ( ! _goingRight )
	    _pacManRect.setX( _pacManRect.x() + _pacManRect.width() - _speed );

	_pacManRect.setWidth( _speed );
	painter->eraseRect( _pacManRect );
    }


    if ( _pos + size > rect.width() )	// Right edge reached?
    {
	// Notice: This can also happen when the rectangle is resized - i.e. it
	// really makes sense to do that right here rather than at the end of
	// this function!

	// Turn left

	_pos 		= rect.width() - size;
	_goingRight	= false;
	_mouth		= _minMouth;
    }
    else if ( _pos < 0 )	// Left edge reached?
    {
	// Turn right

	_pos 		= 0;
	_goingRight	= true;
	_mouth		= _minMouth;
    }


    // Draw PacMan (double-buffered)

    _pacManRect = QRect( 0, 0, size, size );
    QPixmap pixmap( size, size );
    pixmap.fill( painter->backgroundColor() );
    QPainter p( &pixmap, _widget );
    p.setBrush( _brush );

    if ( _goingRight )
    {
	p.drawPie( _pacManRect,
		   _mouth * 16,				// arc (1/16 degrees)
		   ( 360 - 2 * _mouth ) * 16 );		// arc lenght (1/16 degrees)
    }
    else
    {
	p.drawPie( _pacManRect,
		   ( 180 + _mouth ) * 16,		// arc (1/16 degrees)
		   ( 360 - 2 * _mouth ) * 16 );		// arc lenght (1/16 degrees)
    }

    _pacManRect = QRect( rect.x() + _pos,		// x
			 ( rect.height() - size ) / 2,	// y
			 size, size );			// width, height

    // Transfer pixmap into widget

#if 0
    QPoint offset = painter->worldMatrix().map( _pacManRect.topLeft() );
    // kdDebug() << "bitBlt() to " << offset.x() << ", " << offset.y() << endl;
    bitBlt( _widget, offset, &pixmap );
#endif

    painter->drawPixmap( _pacManRect.topLeft(), pixmap );


    // Animate mouth for next turn

    _mouth += _mouthInc;

    if ( _mouth >= _maxMouth )		// max open reached
    {
	_mouth    = _maxMouth;
	_mouthInc = -_mouthInc;		// reverse direction
    }
    else if ( _mouth <= _minMouth )	// min open reached
    {
	_mouth    = _minMouth;
	_mouthInc = -_mouthInc;		// reverse direction
    }


    // Advance position for next turn

    if ( _goingRight )
	_pos += _speed;
    else
	_pos -= _speed;
}






KPacMan::KPacMan( QWidget * 	parent,
		  int 		pacManSize,
		  bool		randomStart )
    : QWidget( parent )
{
    _pacManSize	= pacManSize;
    _pacMan 	= new KPacManAnimation( this, _pacManSize, randomStart );
    _timer	= 0;
    _interval	= 100;	// millisec
    _active	= false;
    _painter	= new QPainter( this );
    _margin	= 1;
}


KPacMan::~KPacMan()
{
    if ( _painter )
	delete _painter;

    if ( _pacMan )
	delete _pacMan;
}


void
KPacMan::start()
{
    if ( ! _timer )
    {
	_timer = new QTimer( this );
    }

    _pacMan->restart();

    if ( _timer )
    {
	_active = true;
	_timer->start( _interval );
	connect( _timer, SIGNAL( timeout() ),
		 this,   SLOT  ( animate() ) );
    }
}


void
KPacMan::stop()
{
    _active = false;

    if ( _timer )
	_timer->stop();

    repaint();
}


void
KPacMan::animate()
{
    repaint( false );
}


void
KPacMan::setInterval( int intervalMilliSec )
{
    _interval = intervalMilliSec;
    _pacMan->setInterval( _interval );

    if ( _timer )
	_timer->changeInterval( _interval );
}


void
KPacMan::paintEvent( QPaintEvent *ev )
{
    if ( _active )
    {
	_pacMan->animate( _painter, QRect( _margin, 0, width() - _margin, height() ) );
    }
    else
    {
	_painter->eraseRect( ev->rect() );
    }
}


void
KPacMan::mouseReleaseEvent ( QMouseEvent *ev )
{
    if ( _active )
    {
	if ( _pacMan->lastPacMan().contains( ev->pos() ) )
	    stop();
    }
}


QSize
KPacMan::sizeHint() const
{
    return QSize( 16 * _pacManSize,		// width - admittedly somewhat random
		  _pacManSize + 2 * _margin );	// height
}



// EOF
