/*
 *   File name:	kstdcleanup.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 *
 *   Updated:	2010-02-02
 */


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <klocale.h>
#include "kcleanup.h"
#include "kstdcleanup.h"

using namespace KDirStat;


KCleanup *
KStdCleanup::openInKonqueror( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_open_in_konqueror",
				      "kfmclient openURL %p",
				      i18n( "Open in &Konqueror" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true );
    cleanup->setWorksForFile    ( true );
    cleanup->setWorksForDotEntry( true );
    cleanup->setWorksLocalOnly	( false );
    cleanup->setRefreshPolicy( KCleanup::noRefresh );
    cleanup->setIcon(KIcon("konqueror.png" ));
    cleanup->setShortcut( Qt::CTRL + Qt::Key_K );
    
    return cleanup;
}


KCleanup *
KStdCleanup::openInTerminal( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_open_in_terminal",
				      "konsole",
				      i18n( "Open in &Terminal" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true );
    cleanup->setWorksForFile    ( true );
    cleanup->setWorksForDotEntry( true );
    cleanup->setRefreshPolicy( KCleanup::noRefresh );
    cleanup->setIcon(KIcon("utilities-terminal" ));
    cleanup->setShortcut( Qt::CTRL + Qt::Key_T );

    return cleanup;
}


KCleanup *
KStdCleanup::compressSubtree( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_compress_subtree",
				      "cd ..; tar cjvf %n.tar.bz2 %n && rm -rf %n",
				      i18n( "&Compress" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true  );
    cleanup->setWorksForFile    ( false );
    cleanup->setWorksForDotEntry( false );
    cleanup->setRefreshPolicy( KCleanup::refreshParent );
    cleanup->setIcon(KIcon( "utilities-file-archiver" ));

    return cleanup;
}


KCleanup *
KStdCleanup::makeClean( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_make_clean",
				      "make clean",
				      i18n( "&make clean" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true  );
    cleanup->setWorksForFile    ( false );
    cleanup->setWorksForDotEntry( true  );
    cleanup->setRefreshPolicy( KCleanup::refreshThis );

    return cleanup;
}


KCleanup *
KStdCleanup::deleteTrash( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_delete_trash",
				      "rm -f *.o *~ *.bak *.auto core",
				      i18n( "Delete T&rash Files" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true  );
    cleanup->setWorksForFile    ( false );
    cleanup->setWorksForDotEntry( true  );
    cleanup->setRefreshPolicy( KCleanup::refreshThis );
    cleanup->setRecurse( true );

    return cleanup;
}


KCleanup *
KStdCleanup::moveToTrashBin( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_move_to_trash_bin",
				      "kfmclient move %p %t",
				      i18n( "Delete (to Trash &Bin)" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true  );
    cleanup->setWorksForFile    ( true  );
    cleanup->setWorksForDotEntry( false );
    cleanup->setRefreshPolicy( KCleanup::assumeDeleted );
    /* The icon standard says the action should be "edit-trash"
       However, Oxygen doesn't have that icon, so I'm setting
       "user-trash" which will probably be the same in most
       icon sets. */
    //cleanup->setIcon(KIcon( "edit-trash" ));
    cleanup->setIcon(KIcon( "user-trash" ));
    cleanup->setShortcut( Qt::CTRL + Qt::Key_X );

    return cleanup;
}
	

KCleanup *
KStdCleanup::hardDelete( KActionCollection *parent )
{
    KCleanup *cleanup = new KCleanup( "cleanup_hard_delete",
				      "rm -rf %p",
				      i18n( "&Delete (no way to undelete!)" ),
				      parent );
    Q_CHECK_PTR( cleanup );
    cleanup->setWorksForDir     ( true  );
    cleanup->setWorksForFile    ( true  );
    cleanup->setWorksForDotEntry( false );
    cleanup->setAskForConfirmation( true );
    cleanup->setRefreshPolicy( KCleanup::assumeDeleted );
    cleanup->setIcon(KIcon( "edit-delete" ));
    cleanup->setShortcut( Qt::CTRL + Qt::Key_Delete );

    return cleanup;
}
	


// EOF
