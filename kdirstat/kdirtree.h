/*
 *   File name:	kdirtree.h
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2001-06-21
 *
 *   $Id: kdirtree.h,v 1.1 2001/06/29 16:37:49 hundhammer Exp $
 *
 */

/*
 * TODO:
 *
 * KFileItemIterator
 * KFileItemSortedIterator
 */

#ifndef KDirTree_h
#define KDirTree_h


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <qqueue.h>
#include <kfileitem.h>
#include <kio/jobclasses.h>

#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif

// Open a new name space since KDE's name space is pretty much cluttered
// already - all names that would even remotely match are already used up,
// yet the resprective classes don't quite fit the purposes required here.

namespace KDirStat
{
    // With today's hard disks, the 2 GB we could sum up with 'long' (or 4 GB
    // with 'unsigned long') are definitely not enough. So we have to go for
    // something larger:
    typedef long long	KFileSize;

    // Taken from Linux <limits.h> (the Alpha definition - 64 Bit long!).
    // This is how much bytes this program can handle.
#define KFileSizeMax 9223372036854775807LL

    // Forward declarations
    class KDirTree;
    class KDirReadJob;
    class KDirTreeView;


    /**
     * Status of a directory read job.
     **/
    typedef enum
    {
	KDirQueued,		// Waiting in the directory read queue
	KDirReading,		// Reading in progress
	KDirFinished,		// Reading finished and OK
	KDirAborted,		// Reading aborted upon user request
	KDirError		// Error while reading
    } KDirReadState;

    typedef enum
    {
	KDirReadLocal,		// use opendir() and lstat()
	KDirReadKDirLister	// use KDE 2.x's KDirLister
    } KDirReadMethod;


    /**
     * The most basic building block of a @ref KDirTree:
     *
     * Information about one single directory entry. This is the type of info
     * typically obtained by stat() / lstat() or similar calls.  Most of this
     * can also be obtained by @ref KIO::KDirListJob, but not all: The device
     * this file resides on is something none of KIO's many classes will tell
     * (since of course this only makes sense for local files) - yet this had
     * been _the_ single most requested feature of KDirStat <1.0: Stay on one
     * filesystem. To facilitate this, information about the device is
     * required, thus we'll do lstat() sys calls ourselves for local
     * files. This is what the classes in this file are all about.
     *
     * This class is tuned for size rather than speed: A typical Linux system
     * easily has 150,000+ file system objects, and at least one entry of this
     * sort is required for each of them.
     *
     * This class provides stubs for children management, yet those stubs all
     * are default implementations that don't really deal with
     * children. Derived classes need to take care of that.
     *
     * @short Basic file information (like obtained by the lstat() sys call)
     **/
    class KFileInfo
    {
    public:
	/**
	 * Default constructor.
	 **/
	KFileInfo(  KFileInfo  * 	parent = 0,
		    const char *	name   = 0 );

	/**
	 * Constructor from a stat buffer (i.e. based on an lstat() call).
	 **/
	KFileInfo( const QString &	filenameWithoutPath,
		   struct stat *	statInfo,
		   KFileInfo   *	parent = 0 );

	/**
	 * Constructor from a KFileItem, i.e. from a @ref KIO::StatJob
	 **/
	KFileInfo( const KFileItem *	fileItem,
		   KFileInfo * 		parent = 0 );

	/**
	 * Destructor.
	 **/
	virtual ~KFileInfo();

	/**
	 * Returns whether or not this is a local file (protocol "file:").
	 * It might as well be a remote file ("ftp:", "smb:" etc.).
	 **/
	bool			isLocalFile()	const { return _isLocalFile; }

	/**
	 * Returns the file or directory name without path, i.e. only the last
	 * path name component (i.e. "printcap" rather than "/etc/printcap").
	 *
	 * If a directory scan doesn't begin at the root directory and this is
	 * the top entry of this directory scan it will also contain the base
	 * path and maybe the protocol (for remote files),
	 * i.e. "/usr/share/man" rather than just "man" if a scan was requested
	 * for "/usr/share/man". Notice, however, that the entry for
	 * "/usr/share/man/man1" will only return "man1" in this example.
	 **/
	QString			name()		const { return _name; }

	/**
	 * Returns the full URL of this object with full path and protocol
	 * (unless the protocol is "file:").
	 *
	 * This is an expensive operation since it will recurse up to the top
	 * of the tree.
	 **/
	QString			url()			const;

	/**
	 * Very much like @ref KFileInfo::url(), but with "/<Files>" appended
	 * if this is a dot entry. Useful for debugging.
	 **/
	QString			debugUrl()		const;

	/**
	 * Returns part no. "level" of this object's URL, i.e. traverses up the
	 * tree until this tree level is reached and returns this predecessor's
	 * @ref name() . This is useful for tree searches in symmetrical trees
	 * to find an item's counterpart in the other tree.
	 **/
	QString			urlPart( int level )	const;

	/**
	 * Returns the major and minor device numbers of the device this file
	 * resides on or 0 if this is a remote file.
	 **/
	dev_t			device()	const { return _device;	}

	/**
	 * The file permissions and object type as returned by lstat().
	 * You might want to use the repective convenience methods instead:
	 * @ref isDir(), @ref isFile(), ...
	 **/
	mode_t			mode()		const { return _mode;	}

	/**
	 * The number of hard links to this file. Relevant for size summaries
	 * to avoid counting one file several times.
	 **/
	nlink_t			links()		const { return _links;	}

	/**
	 * The file size in bytes. This does not take unused space in the last
	 * disk block (cluster) into account, yet it is the only size all kinds
	 * of info functions can obtain.
	 **/
	KFileSize		size()		const { return _size;	}

	/**
	 * The file size in 512 byte blocks.
	 **/
	KFileSize		blocks()	const { return _blocks; }

	/**
	 * The size of one single block that @ref blocks() returns.
	 * Notice: This is _not_ the blocksize that lstat() returns!
	 **/
	KFileSize		blockSize()	const { return 512L;	}

	/**
	 * The modification time of the file (not the inode).
	 **/
	time_t			mtime()		const { return _mtime;	}

	/**
	 * Returns the total size in bytes of this subtree.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual KFileSize	totalSize()	{ return _size;	  }

	/**
	 * Returns the total size in blocks of this subtree.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual KFileSize	totalBlocks()	{ return _blocks; }

	/**
	 * Returns the total number of children in this subtree, excluding this item.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual int		totalItems()	{ return 0;	}

	/**
	 * Returns the total number of subdirectories in this subtree,
	 * excluding this item. Dot entries and "." or ".." are not counted.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual int		totalSubDirs()	{ return 0;	}

	/**
	 * Returns the total number of plain file children in this subtree,
	 * excluding this item.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual int		totalFiles()	{ return 0;	}

	/**
	 * Returns the latest modification time of this subtree.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual time_t		latestMtime()	{ return _mtime;  }

	/**
	 * Returns true if this subtree is finished reading.
	 *
	 * This default implementation always returns 'true';
	 * derived classes should overwrite this.
	 **/
	virtual bool		isFinished() 	{ return true; }
	
	/**
	 * Returns true if this subtree is busy, i.e. it is not finished
	 * reading yet.
	 * 
	 * This default implementation always returns 'false';
	 * derived classes should overwrite this.
	 **/
	virtual bool 		isBusy() 	{ return false; }

	/**
	 * Returns the number of pending read jobs in this subtree. When this
	 * number reaches zero, the entire subtree is done.
	 * Derived classes that have children should overwrite this.
	 **/
	virtual int		pendingReadJobs()	{ return 0;  }
	
	/**
	 * Notification of a new directory read job somewhere in the subtree.
	 *
	 * This default implementation does nothing.
	 * Derived classes might want to overwrite this.
	 **/
	virtual void 		readJobAdded()		{}
	
	/**
	 * Notification of a finished directory read job somewhere in the
	 * subtree.
	 *
	 * This default implementation does nothing.
	 * Derived classes might want to overwrite this.
	 **/
	virtual void 		readJobFinished() {}


	//
	// Tree management
	//

	/**
	 * Returns a pointer to this entry's parent entry or 0 if there is
	 * none.
	 **/
	KFileInfo *	parent()		const { return _parent; }

	/**
	 * Set the "parent" pointer.
	 **/
	void		setParent( KFileInfo *newParent ) { _parent = newParent; }

	/**
	 * Returns a pointer to the next entry on the same level
	 * or 0 if there is none.
	 **/
	KFileInfo *	next()			const { return _next;	}

	/**
	 * Set the "next" pointer.
	 **/
	void		setNext( KFileInfo *newNext ) { _next = newNext; }

	/**
	 * Returns the first child of this item or 0 if there is none.
	 * Use the child's next() method to get the next child.
	 *
	 * This default implementation always returns 0.
	 **/
	virtual KFileInfo * firstChild()	const { return 0;	}

	/**
	 * Set this entry's first child.
	 * Use this method only if you know exactly what you are doing.
	 *
	 * This default implementation does nothing.
	 * Derived classes might want to overwrite this.
	 **/
	virtual void	setFirstChild( KFileInfo *newFirstChild )
	    { NOT_USED( newFirstChild ); }
	
	/**
	 * Returns true if this entry has any children.
	 **/
	virtual bool	hasChildren()		const;

	/**
	 * Locate a child somewhere in this subtree whose URL (i.e. complete
	 * path) matches the URL passed. Returns 0 if there is no such child.
	 *
	 * Notice: This is a very expensive operation since the entire subtree
	 * is searched recursively.
	 *
	 * Derived classes might or might not wish to overwrite this method;
	 * it's only advisable to do so if a derived class comes up with a
	 * different method than brute-force search all children.
	 **/
	virtual KFileInfo * locate( QString url );

	/**
	 * Insert a child into the children list.
	 *
	 * The order of children in this list is absolutely undefined;
	 * don't rely on any implementation-specific order.
	 *
	 * This default implementation does nothing.
	 **/
	virtual void	insertChild( KFileInfo *newChild ) { NOT_USED( newChild ); }

	/**
	 * Return the "Dot Entry" for this node if there is one (or 0
	 * otherwise): This is a pseudo entry that directory nodes use to store
	 * non-directory children separately from directories. This way the end
	 * user can easily tell which summary fields belong to the directory
	 * itself and which are the accumulated values of the entire subtree.
	 *
	 * This default implementation always returns 0.
	 **/
	virtual KFileInfo *dotEntry()	const { return 0; }

	/**
	 * Set a "Dot Entry". This makes sense for directories only.
	 *
	 * This default implementation does nothing.
	 **/
	virtual void	setDotEntry( KFileInfo *newDotEntry ) { NOT_USED( newDotEntry ); }

	/**
	 * Returns true if this is a "Dot Entry".
	 * See @ref dotEntry() for details.
	 *
	 * This default implementation always returns false.
	 **/
	virtual bool	isDotEntry() const { return false; }

	/**
	 * Returns the tree level (depth) of this item.
	 * The topmost level is 0.
	 *
	 * This is an expensive operation since it will recurse up to the top
	 * of the tree.
	 **/
	int		treeLevel() const;

	/**
	 * Notification that a child has been added somewhere in the subtree.
	 *
	 * This default implementation does nothing.
	 **/
	virtual void	childAdded( KFileInfo *newChild ) { NOT_USED( newChild ); }

	/**
	 * Notification that a child is about to be deleted somewhere in the subtree.
	 *
	 * This default implementation does nothing.
	 **/
	virtual void	childDeleted( KFileInfo *deletedChild ) { NOT_USED( deletedChild ); }

	/**
	 * Get the current state of the directory reading process:
	 *
	 * This default implementation always returns KDirFinished.
	 * Derived classes should overwrite this.
	 **/
	virtual KDirReadState readState() const { return KDirFinished; }


	//
	// File type / mode convenience methods.
	// These are simply shortcuts to the respective macros from
	// <sys/stat.h>.
	//

	/**
	 * Returns true if this is a directory.
	 **/
	bool isDir()		const { return S_ISDIR( _mode ) ? true : false; }

	/**
	 * Returns true if this is a regular file.
	 **/
	bool isFile()		const { return S_ISREG( _mode ) ? true : false; }

	/**
	 * Returns true if this is a symbolic link.
	 **/
	bool isSymLink()	const { return S_ISLNK( _mode ) ? true : false; }


	/**
	 * Returns true if this is a (block or character) device.
	 **/
	bool isDevice()		const { return ( S_ISBLK ( _mode ) ||
						 S_ISCHR ( _mode )   ) ? true : false; }

	/**
	 * Returns true if this is a block device.
	 **/
	bool isBlockDevice()	const { return S_ISBLK ( _mode ) ? true : false; }

	/**
	 * Returns true if this is a block device.
	 **/
	bool isCharDevice()	const { return S_ISCHR ( _mode ) ? true : false; }

	/**
	 * Returns true if this is a "special" file, i.e. a (block or character)
	 * device, a FIFO (named pipe) or a socket.
	 **/
	bool isSpecial()	const { return ( S_ISBLK ( _mode ) ||
						 S_ISCHR ( _mode ) ||
						 S_ISFIFO( _mode ) ||
						 S_ISSOCK( _mode )   ) ? true : false; }
    protected:

	// Data members.
	//
	// Keep this short in order to use as little memory as possible -
	// there will be a _lot_ of entries of this kind!

	QString		_name;		// the file name (without path!)
	bool		_isLocalFile;	// flag: local or remote file?
	dev_t		_device;	// device this object resides on
	mode_t		_mode;		// file permissions + object type
	nlink_t		_links;		// number of links
	KFileSize	_size;		// size in bytes
	KFileSize	_blocks;	// 512 bytes blocks
	time_t		_mtime;		// modification time

	KFileInfo *	_parent;	// pointer to the parent entry
	KFileInfo *	_next;		// pointer to the next entry
    };	// class KFileInfo


    /**
     * A more specialized version of @ref KFileInfo: This class can actually
     * manage children. The base class (@ref KFileInfo) has only stubs for the
     * respective methods to integrate seamlessly with the abstraction of a
     * file / directory tree; this class fills those stubs with life.
     *
     * @short directory item within a @ref KDirTree.
     **/
    class KDirInfo: public KFileInfo
    {
    public:
	/**
	 * Default constructor.
	 *
	 * If "asDotEntry" is set, this will be used as the parent's
	 * "dot entry", i.e. the pseudo directory that holds all the parent's
	 * non-directory children. This is the only way to create a "dot
	 * entry"!
	 **/
	KDirInfo( KFileInfo *	parent 		= 0,
		  bool		asDotEntry	= false );

	/**
	 * Constructor from a stat buffer (i.e. based on an lstat() call).
	 **/
	KDirInfo( const QString	& filenameWithoutPath,
		  struct stat	* statInfo,
		  KFileInfo	* parent	= 0 );

	/**
	 * Constructor from a KFileItem, i.e. from a @ref KIO::StatJob
	 **/
	KDirInfo( const KFileItem	* fileItem,
		  KFileInfo		* parent	= 0 );

	/**
	 * Destructor.
	 **/
	virtual ~KDirInfo();


	/**
	 * Returns the total size in bytes of this subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual KFileSize 	totalSize();

	/**
	 * Returns the total size in blocks of this subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual KFileSize 	totalBlocks();

	/**
	 * Returns the total number of children in this subtree, excluding this item.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual int		totalItems();

	/**
	 * Returns the total number of subdirectories in this subtree,
	 * excluding this item. Dot entries and "." or ".." are not counted.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual int		totalSubDirs();

	/**
	 * Returns the total number of plain file children in this subtree,
	 * excluding this item.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual int		totalFiles();

	/**
	 * Returns the latest modification time of this subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual time_t 		latestMtime();

	/**
	 * Returns true if this subtree is finished reading.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual bool		isFinished();
	
	/**
	 * Returns true if this subtree is busy, i.e. it is not finished
	 * reading yet.
	 * 
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual bool 		isBusy();

	/**
	 * Returns the number of pending read jobs in this subtree. When this
	 * number reaches zero, the entire subtree is done.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual int		pendingReadJobs()	{ return _pendingReadJobs;  }

	/**
	 * Returns the first child of this item or 0 if there is none.
	 * Use the child's next() method to get the next child.
	 **/
	virtual KFileInfo * firstChild() const { return _firstChild;	}

	/**
	 * Set this entry's first child.
	 * Use this method only if you know exactly what you are doing.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual void	setFirstChild( KFileInfo *newfirstChild )
	    { _firstChild = newfirstChild; }
	
	/**
	 * Insert a child into the children list.
	 *
	 * The order of children in this list is absolutely undefined;
	 * don't rely on any implementation-specific order.
	 **/
	virtual void insertChild( KFileInfo *newChild );

	/**
	 * Get the "Dot Entry" for this node if there is one (or 0 otherwise):
	 * This is a pseudo entry that directory nodes use to store
	 * non-directory children separately from directories. This way the end
	 * user can easily tell which summary fields belong to the directory
	 * itself and which are the accumulated values of the entire subtree.
	 **/
	virtual KFileInfo * dotEntry()	const { return _dotEntry; }

	/**
	 * Set a "Dot Entry". This makes sense for directories only.
	 **/
	virtual void setDotEntry( KFileInfo *newDotEntry ) { _dotEntry = newDotEntry; }

	/**
	 * Returns true if this is a "Dot Entry". See @ref dotEntry() for
	 * details.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual bool isDotEntry() const { return _isDotEntry; }

	/**
	 * Notification that a child has been added somewhere in the subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual void childAdded( KFileInfo *newChild );

	/**
	 * Notification that a child is about to be deleted somewhere in the
	 * subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual void childDeleted( KFileInfo *deletedChild );

	/**
	 * Notification of a new directory read job somewhere in the subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual void readJobAdded();
	
	/**
	 * Notification of a finished directory read job somewhere in the
	 * subtree.
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual void readJobFinished();

	/**
	 * Finalize this directory level after reading it is completed.
	 * This does _not_ mean reading reading all subdirectories is completed
	 * as well!
	 *
	 * Clean up unneeded dot entries.
	 **/
	virtual void finalizeLocal();

	/**
	 * Get the current state of the directory reading process:
	 *
	 *    KDirQueued	waiting in the directory read queue
	 *    KDirReading	reading in progress
	 *    KDirFinished	reading finished and OK
	 *    KDirAborted	reading aborted upon user request
	 *    KDirError		error while reading
	 *
	 * Reimplemented - inherited from @ref KFileInfo.
	 **/
	virtual KDirReadState readState() const;

	/**
	 * Set the state of the directory reading process.
	 * See @ref readState() for details.
	 **/
	void setReadState( KDirReadState newReadState )
	    { _readState = newReadState; }


    protected:

	/**
	 * Recursively recalculate the summary fields when they are dirty.
	 *
	 * This is a _very_ expensive operation since the entire subtree may
	 * recursively be traversed.
	 **/
	void		recalc();

	/**
	 * Clean up unneeded / undesired dot entries:
	 * Delete dot entries that don't have any children,
	 * reparent dot entry children to the "real" (parent) directory if
	 * there are not subdirectory siblings at the level of the dot entry.
	 **/
	void		cleanupDotEntries();


	bool		_isDotEntry;		// Flag: is this entry a "dot entry"?
	int		_pendingReadJobs;	// number of open directories in this subtree

	// Children management

	KFileInfo *	_firstChild;	// pointer to the first child
	KFileInfo *	_dotEntry;	// pseudo entry to hold non-dir children

	// Some cached values

	KFileSize	_totalSize;
	KFileSize	_totalBlocks;
	int		_totalItems;
	int		_totalSubDirs;
	int		_totalFiles;
	time_t		_latestMtime;

	bool		_summaryDirty;	// dirty flag for the cached values
	KDirReadState	_readState;


    private:

	void init();

    };	// class KDirInfo


    /**
     * A directory read job that can be queued. This is mainly to prevent
     * buffer thrashing because of too many directories opened at the same time
     * because of simultaneous reads or even system resource consumption
     * (directory handles in this case).
     *
     * Objects of this kind are transient by nature: They live only as long as
     * the job is queued or executed. When it's done, the data is contained in
     * the corresponding @ref KDirInfo subtree of the corresponding @ref
     * KDirTree.
     *
     * An abstract base class that reads one directory.
     *
     * For each entry automatically a @ref KFileInfo or @ref KDirInfo will be
     * created and added to the parent @ref KDirInfo. For each directory a new
     * @ref KDirReadJob will be created and added to the @ref KDirTree 's job
     * queue.
     *
     * Notice: This class contains pure virtuals - you cannot use it
     * directly. Derive your own class from it or use one of
     * @ref KLocalDirReadJob or @ref KAnyDirReadJob.
     *
     * @short Abstract base class for directory reading.
     **/
    class KDirReadJob
    {
    public:
	/**
	 * Constructor.
	 **/
	KDirReadJob( KDirTree *tree, KDirInfo *dir );

	/**
	 * Destructor.
	 **/
	virtual ~KDirReadJob();

	/**
	 * Start reading the directory. Prior to this nothing happens.
	 *
	 * Please notice there is no corresponding abortReading() call:
	 * Simply delete the reader if the user requests to abort reading.
	 *
	 * Derived classes need to implement this method.
	 **/
	virtual void startReading() = 0;

	/**
	 * Returns the corresponding @ref KDirInfo item.
	 **/
	virtual KDirInfo * dir() { return _dir; }

    protected:

	/**
	 * Notification that a new child has been added.
	 *
	 * Derived classes are required to call this whenever a new child is
	 * added so this notification can be passed up to the @ref KDirTree
	 * which in turn emits a corresponding signal.
	 **/
	void childAdded( KFileInfo *newChild );

	/**
	 * Notification that a child is about to be deleted.
	 *
	 * Derived classes are required to call this just before a child is
	 * deleted so this notification can be passed up to the @ref KDirTree
	 * which in turn emits a corresponding signal.
	 *
	 * Derived classes are not required to handle child deletion at all,
	 * but if they do, calling this method is required.
	 **/
	void childDeleted( KFileInfo *deletedChild );


	KDirTree *	_tree;
	KDirInfo *	_dir;
    };


    /**
     * Impementation of the abstract @ref KDirReadJob class that reads a local
     * directory.
     *
     * This will use lstat() system calls rather than KDE's network transparent
     * directory services since lstat() unlike the KDE services can obtain
     * information about the device (i.e. file system) a file or directory
     * resides on. This is important if you wish to limit directory scans to
     * one file system - which is most desirable when that one file system runs
     * out of space.
     *
     * @short Directory reader that reads one local directory.
     **/
    class KLocalDirReadJob: public KDirReadJob
    {
    public:
	/**
	 * Constructor.
	 **/
	KLocalDirReadJob( KDirTree * tree, KDirInfo * dir );

	/**
	 * Destructor.
	 **/
	virtual ~KLocalDirReadJob();

	/**
	 * Start reading the directory. Prior to this nothing happens.
	 *
	 * Inherited and reimplemented from @ref KDirReadJob.
	 **/
	virtual void startReading();

    protected:
	DIR * _diskDir;
    };


    /**
     * Generic impementation of the abstract @ref KDirReadJob class, using
     * KDE's network transparent IO methods.
     *
     * This is much more generic than @ref KLocalDirReadJob since it supports
     * protocols like 'ftp', 'http', 'smb', 'tar' etc., too. Its only drawback
     * is that is cannot be prevented from crossing file system boundaries -
     * which makes it pretty useless for figuring out the cause of a 'file
     * system full' error.
     *
     * @short Generic directory reader that reads one directory, remote or local.
     **/
    class KAnyDirReadJob: public QObject, public KDirReadJob
    {
	Q_OBJECT
	
    public:
	/**
	 * Constructor.
	 **/
	KAnyDirReadJob( KDirTree * tree, KDirInfo * dir );

	/**
	 * Destructor.
	 **/
	virtual ~KAnyDirReadJob();

	/**
	 * Start reading the directory. Prior to this nothing happens.
	 *
	 * Inherited and reimplemented from @ref KDirReadJob.
	 **/
	virtual void startReading();

	
    protected slots:
	/**
	 * Receive directory entries from a KIO job.
	 **/
        void entries( KIO::Job *		job,
		      const KIO::UDSEntryList &	entryList );

	/**
	 * KIO job is finished.
	 **/
	void finished( KIO::Job * job );
	
    protected:
	
	KIO::ListJob *	_job;
    };



    /**
     * This class provides some infrastructure as well as global data for a
     * directory tree. It acts as the glue that holds things together: The root
     * item from which to descend into the subtree, the read queue and some
     * global policies (like whether or not to cross file systems while reading
     * directories).
     *
     * @short Directory tree global data and infrastructure
     **/
    class KDirTree: public QObject
    {
	Q_OBJECT

    public:
	/**
	 * Constructor.
	 *
	 * Remember to call @ref startReading() after the constructor and
	 * setting up connections.
	 **/
	KDirTree();

	/**
	 * Destructor.
	 **/
	virtual ~KDirTree();

	/**
	 *
	 **/
	void startReading( const KURL &	url );

	/**
	 * Returns the root item of this tree.
	 */
	KFileInfo *	root() const { return _root; }

	/**
	 * Locate a child somewhere in the tree whose URL (i.e. complete path)
	 * matches the URL passed. Returns 0 if there is no such child.
	 *
	 * Notice: This is a very expensive operation since the entire tree is
	 * searched recursively.
	 *
	 * This is merely a convenience method that maps to
	 *    KDirTree::root()->locate( url )
	 **/
	KFileInfo *	locate( QString url ) { return _root ? _root->locate( url ) : 0; }

	/**
	 * Notification of a finished directory read job.
	 * All read jobs are required to call this upon (successful or
	 * unsuccessful) completion.
	 **/
	void jobFinishedNotify( KDirReadJob *job );

	/**
	 * Add a new directory read job to the queue.
	 **/
	void addJob( KDirReadJob * job );

	/**
	 * Obtain the directory read method for this tree:
	 *    KDirReadLocal		use opendir() and lstat()
	 *    KDirReadKDirLister	use KDE 2.x's KDirLister
	 **/
	KDirReadMethod readMethod() const { return _readMethod; }

	/**
	 * Should directory scans cross file systems?
	 *
	 * Notice: This can only be avoided with local directories where the
	 * device number a file resides on can be obtained.
	 * Remember, that's what this is all about ;-)
	 **/
	bool	crossFileSystems() const { return _crossFileSystems; }

	/**
	 * Set or unset the "cross file systems" flag.
	 **/
	void	setCrossFileSystems( bool doCross ) { _crossFileSystems = doCross; }

	/**
	 * Notification that a child has been added.
	 *
	 * Directory read jobs are required to call this for each child added
	 * so the tree can emit the corresponding @ref childAdded() signal.
	 **/
	virtual void childAddedNotify( KFileInfo *newChild );

	/**
	 * Notification that a child is about to be deleted.
	 *
	 * Directory read jobs are required to call this for each deleted child
	 * so the tree can emit the corresponding @ref childDeleted() signal.
	 **/
	virtual void childDeletedNotify( KFileInfo *deletedChild );

	/**
	 * Send a @ref progressInfo() signal to keep the user entertained while
	 * directories are being read.
	 **/
	void sendProgressInfo( const QString &infoLine );

	/**
	 * Send a @ref finalizeLocal() signal to give views a chance to
	 * finalize the display of this directory level - e.g. clean up dot
	 * entries, set the final "expandable" state etc.
	 **/
	void sendFinalizeLocal( KDirInfo *dir );


    signals:

	/**
	 * Emitted when a child has been added.
	 **/
	void childAdded( KFileInfo *newChild );

	/**
	 * Emitted when a child is about to be deleted.
	 **/
	void childDeleted( KFileInfo *deletedChild );

	/**
	 * Emitted when reading this directory tree is finished.
	 **/
	void finished();

	/**
	 * Emitted when reading a directory is finished.
	 * This does _not_ mean reading all subdirectories is finished, too -
	 * only this directory level is complete!
	 *
	 * WARNING: 'dir' may be 0 if the the tree's root could not be read.
	 *
	 * Use this signal to do similar cleanups like
	 * @ref KDirInfo::finalizeLocal(), e.g. cleaning up unused / undesired
	 * dot entries like in @ref KDirInfo::cleanupDotEntries().
	 **/
	virtual void finalizeLocal( KDirInfo *dir );

	/**
	 * Single line progress information, emitted when the read status
	 * changes - typically when a new directory is being read. Connect to a
	 * status bar etc. to keep the user entertained.
	 **/
	void progressInfo( const QString &infoLine );


    protected slots:

        /**
	 * Time-sliced work procedure to be performed while the application is
	 * in the main loop: Read some directory entries, but relinquish
	 * control back to the application so it can maintain some
	 * responsiveness. This method uses single-shot timers of minimal
	 * duration to activate itself as soon as there are no more user events
	 * to process. Call this only once directly after inserting a read job
	 * into the job queue.
	 **/
        void timeSlicedRead();

	/**
	 * Receive the result from a KIO::StatJob for the root entry, create
	 * that root entry and recursively begin reading the directory tree
	 * from there.
	 *
	 * This is ugly, and it doesn't belong here. :-(
	 **/
	void statRoot( KIO::Job * job );


    protected:

	KFileInfo *		_root;
	QQueue<KDirReadJob>	_jobQueue;
	KDirReadMethod		_readMethod;
	bool			_crossFileSystems;
	bool			_enableLocalFileReader;
    };

}	// namespace KDirStat


#endif // ifndef KDirTree_h


// EOF