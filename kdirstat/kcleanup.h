/*
 *   File name:	kcleanup.h
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2001-12-09
 *
 *   $Id: kcleanup.h,v 1.2 2001/12/10 10:32:58 hundhammer Exp $
 *
 */


#ifndef KCleanup_h
#define KCleanup_h


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif


#include <qdict.h>
#include <qlist.h>
#include <qintdict.h>
#include "kdirtree.h"


namespace KDirStat
{
    /**
     * Cleanup action to be performed for @ref KDirTree items.
     *
     * @short KDirStat cleanup action
     **/

    class KCleanup: public QObject
    {
	// TODO: Derive from KAction
	// TODO: Overwrite KAction::slotActivated()
	// TODO: KCleanupCollection
	
	Q_OBJECT

    public:

	enum RefreshPolicy { noRefresh, refreshThis, refreshParent, assumeDeleted };

	/**
	 * Constructor.
	 **/
	KCleanup( QString		id		= "",
		  QString		command 	= "",
		  QString		title 		= "" );

	/**
	 * Copy Constructor.
	 *
	 * Notice that this is a not quite complete copy constructor: Since
	 * there is no QObject copy constructor, the inherited QObject members
	 * will be constructed with the QObject default constructor. Thus, an
	 * object created with this copy constructor can rely only on its
	 * KCleanup members. This is intended for save/restore operations only,
	 * not for general use. In particular, DO NOT connect an object thus
	 * constructed with signals. The results will be undefined (at best).
	 **/
	KCleanup( const KCleanup &src );

	/**
	 * Assignment operator.
	 *
	 * This will not modify the QObject members, just the KCleanup
	 * members. Just like the copy constructor, this is intended for
	 * save/restore operations, not for general use.
	 **/
	KCleanup &	operator= ( const KCleanup &src );

	/**
	 * Return the KDirTree this cleanup action works on.
	 **/
	KDirTree *	tree()		const { return _tree; }

	/**
	 * Return the ID (name) of this cleanup action as used for setup
	 * files. This ID should be unique within the application.
	 **/
	const QString &	id()		const { return _id; }
   
	/**
	 * Return the command line that will be executed upon calling @ref
	 * KCleanup::execute(). This command line may contain %p for the
	 * complete path of the directory or file concerned or %n for the pure
	 * file or directory name without path.
	 **/
	const QString &	command()	const { return _command; }

	/**
	 * Return the user title of this command as displayed in menus.
	 **/
	const QString &	title()		const { return _title; }

	/**
	 * Return whether or not this cleanup action is generally enabled.
	 **/
	bool enabled()			const { return _enabled; }

	/**
	 * Return whether or not this cleanup action works for this particular
	 * KFileInfo. Checks all the other conditions (enabled(),
	 * worksForDir(), worksForFile(), ...) accordingly.
	 **/
	bool worksFor( KFileInfo *item ) const;

	/**
	 * Return whether or not this cleanup action works for directories,
	 * i.e. whether or not @ref KCleanup::execute() will be successful if
	 * the object passed is a directory.
	 **/
	bool worksForDir()		const { return _worksForDir; }

	/**
	 * Return whether or not this cleanup action works for plain files.
	 **/
	bool worksForFile()		const { return _worksForFile; }

	/**
	 * Return whether or not this cleanup action works for KDirStat's
	 * special 'Dot Entry' items, i.e. the pseudo nodes created in most
	 * directories that hold the plain files.
	 **/
	bool worksForDotEntry()		const { return _worksForDotEntry; }

	/**
	 * Return whether or not the cleanup action should be performed
	 * recursively in subdirectories of the initial KFileInfo.
	 **/
	bool recurse()			const { return _recurse; }

	/**
	 * Return the refresh policy of this cleanup action - i.e. the action
	 * to perform after each call to KCleanup::execute(). This is supposed
	 * to bring the corresponding KDirTree back into sync after the cleanup
	 * action - the underlying file tree might have changed due to that
	 * cleanup action.
	 *
	 * noRefresh: Don't refresh anything. Assume nothing has changed.
	 * This is the default.
	 *
	 * refreshThis: Refresh the KDirTree from the item on that was passed
	 * to KCleanup::execute().
	 *
	 * refreshParent: Refresh the KDirTree from the parent of the item on
	 * that was passed to KCleanup::execute(). If there is no such parent,
	 * refresh the entire tree.
	 *
	 * assumeDeleted: Do not actually refresh the KDirTree.  Instead,
	 * blindly assume the cleanup action has deleted the item that was
	 * passed to KCleanup::execute() and delete the corresponding subtree
	 * in the KDirTree accordingly. This will work well for most deleting
	 * actions as long as they can be performed without problems. If there
	 * are any problems, however, the KDirTree might easily run out of sync
	 * with the directory tree: The KDirTree will show the subtree as
	 * deleted (i.e. it will not show it any more), but it still exists on
	 * disk. This is the tradeoff to a very quick response. On the other
	 * hand, the user can easily at any time hit one of the explicit
	 * refresh buttons and everything will be back into sync again.
	 **/
	enum RefreshPolicy refreshPolicy()	const { return _refreshPolicy; }


	void setTree			( KDirTree *tree	)	{ _tree			= tree;		}
	void setId			( const QString &id	) 	{ _id			= id;		}
	void setCommand			( const QString &command) 	{ _command 		= command;	}
	void setTitle			( const QString &title	) 	{ _title		= title;	}
	void setEnabled			( bool enabled	)		{ _enabled		= enabled;	}
	void setWorksForDir		( bool canDo 	)		{ _worksForDir		= canDo; 	}
	void setWorksForFile		( bool canDo 	)		{ _worksForFile		= canDo; 	}
	void setWorksForDotEntry	( bool canDo 	)		{ _worksForDotEntry	= canDo; 	}
	void setRecurse			( bool recurse	)		{ _recurse		= recurse; 	}
	void msetRefreshPolicy		( enum RefreshPolicy refreshPolicy ) { _refreshPolicy = refreshPolicy; 	}


	/**
	 * Read all values for this cleanup action from the application's
	 * KConfig object, i.e. from the configuration file. The cleanup action
	 * object will use its ID as a config key, so at least the ID needs to
	 * be valid.
	 **/
	void readConfig();

	/**
	 * Write all of this cleanup action object's values to the
	 * application's KConfig object, i.e. to the configuration file. Just
	 * like in @ref KCleanup::readConfig(), the ID will be used as the
	 * config key.
	 **/
	void writeConfig() const;
   

    public slots:

	/**
	 * The heart of the matter: Perform the cleanup with the KFileInfo
	 * specified.
	 **/
        void execute( KFileInfo *item );

	/**
	 * Perform the cleanup with the current KDirTree selection if there is
	 * any.
	 **/
	void executeWithSelection();
   

    protected:

	/**
	 * Recursively perform the cleanup.
	 **/
	void executeRecursive( KFileInfo *item );

	/**
	 * Retrieve the directory part of a KFileInfo's path.
	 **/
	const QString itemDir( const KFileInfo *item ) const;

	/**
	 * Expand some variables in string 'unexpanded' to information from
	 * within 'item'. Multiple expansion is performed as needed, i.e. the
	 * string may contain more than one variable to expand.  The resulting
	 * string is returned.
	 *
	 * %p expands to item->path(), i.e. the item's full path name.
	 *
	 * /usr/local/bin		for that directory
	 * /usr/local/bin/doit	for a file within it
	 *
	 * %n expands to item->name(), i.e. the last component of the pathname.
	 * The examples above would expand to:
	 *
	 * bin
	 * doit
	 *
	 * For commands that are to be executed from within the 'Clean up'
	 * menu, you might specify something like:
	 *
	 * "kfmclient openURL %p"
	 * "tar czvf %{name}.tgz && rm -rf %{name}"
	 **/
	QString	expandVariables ( const KFileInfo *	item,
				  const QString &	unexpanded ) const;

	/**
	 * Run a command with 'item' as base to expand variables.
	 **/
	void	runCommand	( const KFileInfo *	item,
				  const QString &	command ) const;


	KDirTree *		_tree;
	QString			_id;
	QString			_command;
	QString			_title;
	bool			_enabled;
	bool			_worksForDir;
	bool			_worksForFile;
	bool			_worksForDotEntry;
	bool			_recurse;
	enum RefreshPolicy	_refreshPolicy;
    };


    typedef QDict<KCleanup>		KCleanupDict;
    typedef QDictIterator<KCleanup>	KCleanupDictIterator;

    typedef QIntDict<KCleanup>		KCleanupIntDict;
    typedef QIntDictIterator<KCleanup>	KCleanupIntDictIterator;

    typedef QList<KCleanup>		KCleanupList;
    typedef QListIterator<KCleanup>	KCleanupListIterator;
    
}	// namespace KDirStat


#endif // ifndef KCleanup_h


// EOF
