#pragma once
/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 */

#include "kfileinfo.h"
#include <kfileitem.h>

#ifndef NOT_USED
#define NOT_USED(PARAM) ((void)(PARAM))
#endif

// Open a new name space since KDE's name space is pretty much cluttered
// already - all names that would even remotely match are already used up,
// yet the resprective classes don't quite fit the purposes required here.

namespace KDirStat {
// Forward declarations
class KDirTree;

/**
 * A more specialized version of @ref KFileInfo: This class can actually
 * manage children. The base class (@ref KFileInfo) has only stubs for the
 * respective methods to integrate seamlessly with the abstraction of a
 * file / directory tree; this class fills those stubs with life.
 *
 * @short directory item within a @ref KDirTree.
 **/
class KDirInfo : public KFileInfo {
public:
  /**
   * Default constructor.
   *
   * If "asDotEntry" is set, this will be used as the parent's
   * "dot entry", i.e. the pseudo directory that holds all the parent's
   * non-directory children. This is the only way to create a "dot
   * entry"!
   **/
  KDirInfo(KDirInfo *parent = nullptr, bool asDotEntry = false);

  /**
   * Constructor from a stat buffer (i.e. based on an lstat() call).
   **/
  KDirInfo(const QString &filenameWithoutPath, struct stat *statInfo,
           KDirInfo *parent = nullptr);

  /**
   * Constructor from a KFileItem, i.e. from a @ref KIO::StatJob
   **/
  KDirInfo(const KFileItem *fileItem, KDirInfo *parent = nullptr);

  /**
   * Constructor from the bare neccessary fields
   * for use from a cache file reader
   **/
  KDirInfo(KDirInfo *parent, const QString &filenameWithoutPath,
           mode_t mode, KFileSize size, time_t mtime);

  /**
   * Destructor.
   **/
  virtual ~KDirInfo();

  /**
   * Returns the total size in bytes of this subtree.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  KFileSize totalSize() override;

  /**
   * Returns the total number of children in this subtree, excluding this item.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  int totalItems() override;

  /**
   * Returns the total number of subdirectories in this subtree,
   * excluding this item. Dot entries and "." or ".." are not counted.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  int totalSubDirs() override;

  /**
   * Returns the total number of plain file children in this subtree,
   * excluding this item.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  int totalFiles() override;

  /**
   * Returns the latest modification time of this subtree.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  time_t latestMtime() override;

  /**
   * Returns 'true' if this had been excluded while reading.
   **/
  bool isExcluded() const override { return _isExcluded; }

  /**
   * Set the 'excluded' status.
   **/
  void setExcluded(bool excl = true) override { _isExcluded = excl; }

  /**
   * Returns whether or not this is a mount point.
   *
   * This will return 'false' only if this information can be obtained at
   * all, i.e. if local directory reading methods are used.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  bool isMountPoint() override { return _isMountPoint; }

  /**
   * Sets the mount point state, i.e. whether or not this is a mount
   * point.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  void setMountPoint(bool isMountPoint = true) override;

  /**
   * Returns true if this subtree is finished reading.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  bool isFinished() override;

  /**
   * Returns true if this subtree is busy, i.e. it is not finished
   * reading yet.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  bool isBusy() override;

  /**
   * Returns the number of pending read jobs in this subtree. When this
   * number reaches zero, the entire subtree is done.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  int pendingReadJobs() override { return _pendingReadJobs; }

  size_t numChildren() const override { return children_.size(); }
  KFileInfo * child(size_t i) override { return children_[i]; }

  /**
   * Insert a child into the children list.
   *
   * The order of children in this list is absolutely undefined;
   * don't rely on any implementation-specific order.
   **/
  void insertChild(KFileInfo *newChild) override;

  /**
   * Get the "Dot Entry" for this node if there is one (or 0 otherwise):
   * This is a pseudo entry that directory nodes use to store
   * non-directory children separately from directories. This way the end
   * user can easily tell which summary fields belong to the directory
   * itself and which are the accumulated values of the entire subtree.
   **/
  KDirInfo *dotEntry() const override { return _dotEntry; }

  /**
   * Set a "Dot Entry". This makes sense for directories only.
   **/
  void setDotEntry(KDirInfo *newDotEntry) override { _dotEntry = newDotEntry; }

  /**
   * Returns true if this is a "Dot Entry". See @ref dotEntry() for
   * details.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  bool isDotEntry() const override { return _isDotEntry; }

  /**
   * Notification that a child has been added somewhere in the subtree.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  void childAdded(KFileInfo *newChild) override;

  /**
   * Notification that a child is about to be deleted somewhere in the
   * subtree.
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  void deletingChild(KFileInfo *deletedChild) override;

  /**
   * Notification of a new directory read job somewhere in the subtree.
   **/
  void readJobAdded();

  /**
   * Notification of a finished directory read job somewhere in the
   * subtree.
   **/
  void readJobFinished();

  /**
   * Notification of an aborted directory read job somewhere in the
   * subtree.
   **/
  void readJobAborted();

  /**
   * Finalize this directory level after reading it is completed.
   * This does _not_ mean reading reading all subdirectories is completed
   * as well!
   *
   * Clean up unneeded dot entries.
   **/
  virtual void finalizeLocal();

  /**
   * Recursively finalize all directories from here on -
   * call finalizeLocal() recursively.
   **/
  void finalizeAll(KDirTree *);

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
  KDirReadState readState() const override;

  /**
   * Set the state of the directory reading process.
   * See @ref readState() for details.
   **/
  void setReadState(KDirReadState newReadState);

  /**
   * Returns true if this is a @ref KDirInfo object.
   *
   * Don't confuse this with @ref isDir() which tells whether or not this
   * is a disk directory! Both should return the same, but you'll never
   * know - better be safe than sorry!
   *
   * Reimplemented - inherited from @ref KFileInfo.
   **/
  bool isDirInfo() const override { return true; }

protected:
  /**
   * Recursively recalculate the summary fields when they are dirty.
   *
   * This is a _very_ expensive operation since the entire subtree may
   * recursively be traversed.
   **/
  void recalc();

  /**
   * Clean up unneeded / undesired dot entries:
   * Delete dot entries that don't have any children,
   * reparent dot entry children to the "real" (parent) directory if
   * there are not subdirectory siblings at the level of the dot entry.
   **/
  void cleanupDotEntries();

  //
  // Data members
  //

  bool _isDotEntry : 1;   // Flag: is this entry a "dot entry"?
  bool _isMountPoint : 1; // Flag: is this a mount point?
  bool _isExcluded : 1;   // Flag: was this directory excluded?
  int _pendingReadJobs;   // number of open directories in this subtree
  KDirInfo *_dotEntry;   // pseudo entry to hold non-dir children

  // Some cached values

  KFileSize _totalSize;
  int _totalItems;
  int _totalSubDirs;
  int _totalFiles;
  time_t _latestMtime;

  bool _summaryDirty : 1; // dirty flag for the cached values
  bool _beingDestroyed : 1;
  KDirReadState _readState;

private:
  void recalcOneChild(KFileInfo*);
  void init();
  //TODO: could we store KFileInfo instead of KFileInfo* ?
  std::vector<KFileInfo*> children_;

}; // class KDirInfo

} // namespace KDirStat

