/*
 *   File name:	kcleanup.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2001-12-09
 *
 *   $Id: kcleanup.cpp,v 1.2 2001/12/10 10:32:58 hundhammer Exp $
 *
 */


#include "kcleanup.h"
#include "kdirsaver.h"
#include <kapp.h>
#include <kprocess.h>
#include <kdebug.h>
#include <qapplication.h>
#include <qregexp.h>
#include <stdlib.h>

#define VERBOSE_RUN_COMMAND	1
#define SIMULATE_COMMAND	1

using namespace KDirStat;


KCleanup::KCleanup( QString	id,
		     QString	command,
		     QString	title	)
    : QObject()
    , _id	( id	  )
    , _command	( command )
    , _title	( title	  )
{
    _tree		= 0;
    _enabled		= true;
    _worksForDir	= true;
    _worksForFile	= false;
    _worksForDotEntry	= false;
    _recurse		= false;
    _refreshPolicy	= noRefresh;
}


KCleanup::KCleanup( const KCleanup &src )
    : QObject()
{
    _tree		= src.tree();
    _id			= src.id();
    _command		= src.command();
    _title		= src.title();
    _enabled		= src.enabled();
    _worksForDir	= src.worksForDir();
    _worksForFile	= src.worksForFile();
    _worksForDotEntry	= src.worksForDotEntry();
    _recurse		= src.recurse();
    _refreshPolicy	= src.refreshPolicy();
}


KCleanup &
KCleanup::operator= ( const KCleanup &src )
{
    _tree		= src.tree();
    _id			= src.id();
    _command		= src.command();
    _title		= src.title();
    _enabled		= src.enabled();
    _worksForDir	= src.worksForDir();
    _worksForFile	= src.worksForFile();
    _worksForDotEntry	= src.worksForDotEntry();
    _recurse		= src.recurse();
    _refreshPolicy	= src.refreshPolicy();

    return *this;
}


bool
KCleanup::worksFor( KFileInfo *item ) const
{
    if ( ! _enabled || ! item )
	return false;

    if	( item->isDotEntry() )	return worksForDotEntry();
    if	( item->isDir() )	return worksForDir();
    
    return worksForFile();
}


void
KCleanup::executeWithSelection()
{
    if ( ! _tree )
    {
	kdError() << "Trying to execute cleanup '" << _title
		  << "' without a tree to work on" << endl;
	return;
    }

    if ( ! _tree->selection() )
    {
	kdError() << "Trying to execute cleanup '" << _title
		  << "' without selected item" << endl;
	return;
    }
    
    execute( _tree->selection() );
}


void
KCleanup::execute( KFileInfo *item )
{
    if ( worksFor( item ) )
    {
#if 0
	// FIXME
	KFileInfo *sel  = _tree->selection();
#endif
	
	executeRecursive( item );

	
#if 0
	// FIXME
	// FIXME
	// FIXME
	switch ( _refreshPolicy )
	{
	    case noRefresh:
		// Do nothing.
		break;


	    case refreshThis:
		(void) _tree->refreshSubTree( item );
		break;


	    case refreshParent:
		if ( item->parent() )
		    (void) _tree->refreshSubTree( item->parent() );
		else
		    (void) _tree->refreshAll();

		break;


	    case assumeDeleted:

		// As long as the pointers are still valid (i.e. before
		// deleteSubTree()), try to figure out a good value for a
		// new selection. Try the next sibling or, if there is
		// none, the parent of the current selection.

		KFileInfo *nextSelection = (KFileInfo *) sel->next();
		if ( ! nextSelection )
		    nextSelection = sel->parent();


		// Assume the cleanup action has deleted the item.
		// Modify the KDirTree accordingly.

		_tree->deleteSubTree( item );


		// In case the item we just performed the cleanup action
		// had been the current selection of its KDirTree, try
		// a reasonable automatic selection. This is for cases
		// where the user wants to go through a subtree (maybe
		// even using the keyboard) and deletes an item every now
		// and then. We don't want him to lose his context just
		// because the item he had selected previously now is
		// gone. So we automatically select the logical "next in
		// line" so he can seamlessly continue with his work.
		//
		// Notice: Losing the selection would almost certainly
		// require mouse interaction which might be unacceptable
		// in such a situation.

		if ( sel == item && nextSelection )
		    _tree->selectItem( nextSelection );

		break;
	}
	// FIXME
	// FIXME
	// FIXME
#endif
    }
}


void
KCleanup::executeRecursive( KFileInfo *item )
{
    if ( worksFor( item ) )
    {
	if ( _recurse )
	{
	    // Recurse into all subdirectories.

	    KFileInfo * subdir = item->firstChild();

	    while ( subdir )
	    {
		if ( subdir->isDir() )
		{
		    /**
		     * Recursively execute in this subdirectory, but only if it
		     * really is a directory: File children might have been
		     * reparented to the directory (normally, they reside in
		     * the dot entry) if there are no real subdirectories on
		     * this directory level.
		     **/
		    executeRecursive( subdir );
		}
		subdir = subdir->next();
	    }
	}


	// Perform cleanup for this directory.

	runCommand( item, _command );
    }
}


const QString
KCleanup::itemDir( const KFileInfo *item ) const
{
    QString dir = item->url();

    if ( ! item->isDir() && ! item->isDotEntry() )
    {
	dir.replace ( QRegExp ( "/[^/]*$" ), "" );
    }

    return dir;
}


QString
KCleanup::expandVariables( const KFileInfo *	item,
			   const QString &	unexpanded ) const
{
    QString expanded = unexpanded;

    expanded.replace ( QRegExp ( "%p"	), item->url()  );
    expanded.replace ( QRegExp ( "%n"	), item->name() );

    return expanded;
}


void
KCleanup::runCommand ( const KFileInfo *	item,
		       const QString &		command ) const
{
    KProcess	proc;
    KDirSaver	dir( itemDir( item ) );
    QString	cmd( expandVariables( item, command ) );

#if VERBOSE_RUN_COMMAND
    printf( "\ncd " );				fflush( stdout );
    system( "pwd" );
    printf( "%s\n", (const char *) cmd );	fflush( stdout );
#endif

#if ! SIMULATE_COMMAND
    proc << "sh";
    proc << "-c";
    proc << cmd;

    switch ( _refreshPolicy )
    {
	case noRefresh:
	case assumeDeleted:

	    // In either case it is no use waiting for the command to
	    // finish, so we are starting the command as a pure
	    // background process.

	    proc.start( KProcess::DontCare );
	    break;


	case refreshThis:
	case refreshParent:

	    // If a display refresh is due after the command, we need to
	    // wait for the command to be finished in order to avoid
	    // performing the update prematurely, so we are starting this
	    // process in blocking mode.

	    QApplication::setOverrideCursor( waitCursor );
	    proc.start( KProcess::Block );
	    QApplication::restoreOverrideCursor();
	    break;
    }

#endif
}


void
KCleanup::readConfig()
{
#if 0
    // FIXME
    // FIXME
    // FIXME
    
    KConfig *config = kapp->getConfig();
    KConfigGroupSaver saver( config, _id );

    bool valid 		= config->readBoolEntry( "valid", false	);

    // If the config section requested exists, it should contain a
    // "valid" field with a true value. If not, there is no such
    // section within the config file. In this case, just leave this
    // cleanup action undisturbed - we'd rather have a good default
    // value (as provided - hopefully - by our application upon
    // startup) than a generic empty cleanup action.
   
    if ( valid )
    {
	_command		= config->readEntry	( "command"		);
	_title			= config->readEntry	( "title"		);
	_enabled		= config->readBoolEntry ( "enabled"		);
	_worksForDir		= config->readBoolEntry ( "worksForDir"		);
	_worksForFile		= config->readBoolEntry ( "worksForFile"	);
	_worksForDotEntry	= config->readBoolEntry ( "worksForDotEntry"	);
	_recurse		= config->readBoolEntry ( "recurse"		);
	_refreshPolicy		= (KCleanup::RefreshPolicy) config->readNumEntry( "refreshPolicy" );
    }

    // FIXME
    // FIXME
    // FIXME
#endif
}


void
KCleanup::writeConfig() const
{
#if 0
    // FIXME
    // FIXME
    // FIXME
    
    KConfig *config = kapp->getConfig();
    KConfigGroupSaver saver( config, _id );

    config->writeEntry( "valid",		true			);
    config->writeEntry( "command",		_command		);
    config->writeEntry( "title",		_title			);
    config->writeEntry( "enabled",		_enabled		);
    config->writeEntry( "worksForDir",		_worksForDir		);
    config->writeEntry( "worksForFile",		_worksForFile		);
    config->writeEntry( "worksForDotEntry",	_worksForDotEntry	);
    config->writeEntry( "recurse",		_recurse		);
    config->writeEntry( "refreshPolicy",	(int) _refreshPolicy	);
    
    // FIXME
    // FIXME
    // FIXME
#endif
}


// EOF
