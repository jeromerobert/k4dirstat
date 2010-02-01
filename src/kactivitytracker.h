/*
 *   File name:	kactivitytracker.h
 *   Summary:	Utility object to track user activity
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2007-02-11
 */


#ifndef KActivityTracker_h
#define KActivityTracker_h


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <qobject.h>


/**
 * Helper class to track user activity of any kind: When the user uses an
 * application's actions (menu items etc.), those actions notify this object of
 * that fact. Each action has an amount of "activity points" assigned
 * (i.e. what this action is "worth"). Those points are summed up here, and
 * when a certain number of points is reached, a signal is triggered. This
 * signal can be used for example to ask the user if he wouldn't like to rate
 * this program - or register it if this is a shareware program.
 *
 * @short User activity tracker
 **/
class KActivityTracker: public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor. The ID is a name for the KConfig object to look in for
     * accumulated activity points so far. 'initialThreshold' is only used if
     * the application's @ref KConfig object doesn't contain a corresponding
     * entry yet.
     **/
    KActivityTracker( QObject * 	parent,
		      const QString &	id,
		      long		initialThreshold );

    /**
     * Destructor.
     **/
    virtual ~KActivityTracker();

    /**
     * Returns the number of activity points accumulated so far.
     **/
    long sum() const { return _sum; }

    /**
     * Sets the activity threshold, i.e. when a signal will be sent.
     **/
    void setThreshold( long threshold );
    
    /**
     * Returns the current threshold.
     **/
    long threshold() const { return _threshold; }

    /**
     * Check the sum of activity points accumulated so far against the current
     * threshold and emit a signal if appropriate.
     **/
    void checkThreshold();

    
public slots:

    /**
     * Track an activity, i.e. add the specified amount of activity points to
     * the accumulated sum.
     **/
    void trackActivity( int points );

    /**
     * Set the threshold to its double value.
     **/
    void doubleThreshold() { setThreshold( 2 * threshold() ); }

    
signals:

    /**
     * Emitted when the activity threshold is reached.
     *
     * You might want to set the threshold to a new value when this signal is
     * emitted. You can simply connect it to @ref doubleThreshold().
     **/
    void thresholdReached( void );


protected:

    long	_sum;
    long	_threshold;
    long	_lastSignal;
    QString 	_id;
};

#endif // KActivityTracker_h


// EOF
