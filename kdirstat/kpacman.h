/*
 *   File name:	kpacman.h
 *   Summary:	PacMan animation inside widgets
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2004-03-29
 */


#ifndef KPacMan_h
#define KPacMan_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qdatetime.h>


#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif

class QTimer;


/**
 * Helper class to display a PacMan animation inside a widget.
 * Note that this is not a widget itself, it needs to be placed inside a widget
 * - which fact makes it suitable for use inside non-widget objects such as
 * @ref QListViewItem.
 *
 * If you are looking for a widget that can do all that self-contained, see
 * @ref KPacMan.
 *
 * @short PacMan animation
 **/
class KPacManAnimation
{
public:
    /**
     * Constructor.
     *
     * Create a PacMan sprite in 'widget' of 'size' pixels diameter.  Start at
     * a random position and move in random direction if 'randomStart' is true.
     **/
    KPacManAnimation( QWidget *	widget,
		      int	size,
		      bool	randomStart );

    /**
     * Destructor.
     **/
    virtual ~KPacManAnimation();

    /**
     * Animate PacMan inside this rectangle.
     * Call this frequently enough (e.g. by a timer) to get fluid motion.
     * Set up the painter prior to calling this; the entire rectangle will be
     * cleared with the current brush, and PacMan's outline will be drawn with
     * the current pen.
     *
     * PacMan moves from the left side of this rectangle to the right, turning
     * around when it (he?) reaches the right edge. It (he?) is centered
     * vertically.
     *
     * My, what is the sex of that thing? ;-)
     **/
    void animate( QPainter *	painter,
		  QRect		rect );

    /**
     * Restart - reset to initial position and direction.
     **/
    void restart();

    /**
     * Return the rectangle where the last PacMan was painted.
     **/
    QRect lastPacMan()					{ return _pacManRect; }

    /**
     * Set the animation interval in milliseconds.
     **/
    void	setInterval( int intervalMilliSec ) 	{ _interval = intervalMilliSec; }
    int		interval() const			{ return _interval; }

    /**
     * Number of pixels to move for each phase.
     **/
    int		speed() const				{ return _speed; }
    void	setSpeed( int speed )			{ _speed = speed; }

    /**
     * Brush to draw PacMan's inside. Bright yellow by default.
     **/
    QBrush	brush() const				{ return _brush; }
    void	setBrush( const QBrush & brush )	{ _brush = brush; }

    /**
     * Number of degrees PacMan's mouth opens or closes for each animation.
     **/
    int		mouthOpenInc() const			{ return _mouthInc; }
    void	setMouthOpenInc( int deg ) 		{ _mouthInc = deg; }

    /**
     * Minimum angle in degrees that PacMan's mouth opens.
     **/
    int		minMouthOpenAngle() const		{ return _minMouth;	}
    void 	setMinMouthOpenAngle( int deg )		{ _minMouth = deg;	}

    /**
     * Maximum angle in degrees that PacMan's mouth opens.
     **/
    int		maxMouthOpenAngle() const		{ return _maxMouth;	}
    void	setMaxMouthOpenAngle( int deg )		{ _maxMouth = deg;	}

protected:

    QWidget *	_widget;
    QBrush	_brush;
    QTime	_time;
    QRect	_pacManRect;
    int		_size;
    bool	_randomStart;
    int		_speed;

    int		_minMouth;
    int		_maxMouth;
    int		_mouthInc;
    int		_interval;	// milliseconds


    // Current values

    int		_pos;
    int		_mouth;
    bool	_justStarted;
    bool	_goingRight;
};



/**
 * Widget that displays a PacMan animation.
 *
 * @short PacMan widget
 **/
class KPacMan: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param pacManSize	size of the PacMan sprite
     * @param randomStart	random start position and direction if true
     **/
    KPacMan( QWidget * 	parent		= 0,
	     int 	pacManSize	= 16,
	     bool	randomStart	= false,
	     const char * widgetName	= 0 );

    /**
     * Destructor.
     **/
    virtual ~KPacMan();

    /**
     * Access to the internal @ref PacManAnimation to avoid duplicating all its
     * methods.
     **/
    KPacManAnimation * pacMan() 	{ return _pacMan; }

    /**
     * Access to the internal @ref QPainter to avoid duplicating all its
     * methods. Change this painter in order to change the visual appearance of
     * the PacMan sprite.
     **/
    QPainter *	painter() 		{ return _painter; }

    /**
     * Returns the animation interval in milliseconds.
     **/
    int		interval() const	{ return _interval; }

    /**
     * Set the animation interval in milliseconds.
     **/
    void 	setInterval( int intervalMilliSec );

    /**
     * Return the (left and right) margin.
     **/
    int		margin() 		{ return _margin; }

    /**
     * Set the (left and right) margin - a place PacMan never goes.
     **/
    void	setMargin( int margin )	{ _margin = margin; }

    /**
     * Returns the widget's preferred size.
     *
     * Reimplemented from @ref QWidget.
     **/
    virtual QSize sizeHint() const;


public slots:

    /**
     * Start the animation.
     **/
    void	start();

    /**
     * Stop the animation and clear the widget.
     **/
    void 	stop();

    /**
     * Do one animation. Triggered by timer.
     **/
    void	animate();


protected:

    /**
     * Actually do the painting.
     *
     * Reimplemented from @ref QWidget.
     **/
    virtual void paintEvent( QPaintEvent *ev );

    /**
     * Stop animation on mouse click.
     *
     * Reimplemented from @ref QWidget.
     **/
    virtual void mouseReleaseEvent ( QMouseEvent *ev );


protected:

    KPacManAnimation *	_pacMan;
    QPainter *		_painter;
    QTimer *		_timer;
    int			_interval;	// millisec
    bool		_active;
    int			_margin;
    int			_pacManSize;
};

#endif // KPacMan_h


// EOF
