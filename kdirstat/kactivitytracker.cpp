
/*
 *   File name:	kactivitytracker.cpp
 *   Summary:	Utility object to track user activity
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-02-09
 *
 *   $Id: kactivitytracker.cpp,v 1.1 2002/02/11 10:04:33 hundhammer Exp $
 *
 */



#include <kapp.h>
#include <kdebug.h>
#include <kconfig.h>
#include "kactivitytracker.h"


KActivityTracker::KActivityTracker( QObject *		parent,
				    const QString &	id,
				    long		initialThreshold )
    : QObject( parent )
{
    _id		= id;
    
    KConfig * config = kapp->config();
    config->setGroup( _id );
    _sum	= config->readNumEntry( "activityPoints", 0 );
    _lastSignal = config->readNumEntry( "lastSignal"	, 0 );
    _threshold	= config->readNumEntry( "threshold", initialThreshold );
}


KActivityTracker::~KActivityTracker()
{
    // NOP
}


void
KActivityTracker::setThreshold( long threshold )
{
    _threshold = threshold;
    
    KConfig * config = kapp->config();
    config->setGroup( _id );
    config->writeEntry( "threshold", _threshold );

    checkThreshold();
}


void
KActivityTracker::trackActivity( int points )
{
    _sum += points;

    if ( _sum < 0 )	// handle long int overflow
	_sum = 0;

#if 0
    kdDebug() << "Adding " << points << " activity points."
	      << " Total: " << _sum << " threshold: " << _threshold
	      << endl;
#endif

    KConfig * config = kapp->config();
    config->setGroup( _id );
    config->writeEntry( "activityPoints", _sum );

    checkThreshold();
}


void
KActivityTracker::checkThreshold()
{
    if ( _sum > _threshold && _lastSignal < _threshold )
    {
	// kdDebug() << "Activity threshold reached for " << _id << endl;

	_lastSignal = _sum;
	KConfig * config = kapp->config();
	config->setGroup( _id );
	config->writeEntry( "lastSignal", _lastSignal );
	
	emit thresholdReached();
    }
}




// EOF
