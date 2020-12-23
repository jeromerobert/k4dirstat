/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include "kdirinfo.h"
#include "kdirtree.h"
#include <QDebug>

using namespace KDirStat;

KDirInfo::KDirInfo(KDirInfo *parent, bool asDotEntry)
    : KFileInfo(parent) {
  init();

  if (asDotEntry) {
    _isDotEntry = true;
    _dotEntry = 0;
    _name = ".";
  } else {
    _isDotEntry = false;
    _dotEntry = new KDirInfo(this, true);
  }
}

KDirInfo::KDirInfo(const QString &filenameWithoutPath, struct stat *statInfo,
                   KDirInfo *parent)
    : KFileInfo(filenameWithoutPath, statInfo, parent) {
  init();
  _dotEntry = new KDirInfo(this, true);
}

KDirInfo::KDirInfo(const KFileItem *fileItem, KDirInfo *parent)
    : KFileInfo(fileItem, parent) {
  init();
  _dotEntry = new KDirInfo(this, true);
}

KDirInfo::KDirInfo(KDirInfo *parent,
                   const QString &filenameWithoutPath, mode_t mode,
                   KFileSize size, time_t mtime)
    : KFileInfo(parent, filenameWithoutPath, mode, size, mtime) {
  init();
  _dotEntry = new KDirInfo(this, true);
}

void KDirInfo::init() {
  _isDotEntry = false;
  _pendingReadJobs = 0;
  _dotEntry = 0;
  _totalSize = _size;
  _totalBlocks = _blocks;
  _totalItems = 0;
  _totalSubDirs = 0;
  _totalFiles = 0;
  _latestMtime = _mtime;
  _isMountPoint = false;
  _isExcluded = false;
  _summaryDirty = false;
  _beingDestroyed = false;
  _readState = KDirQueued;
}

KDirInfo::~KDirInfo() {
  _beingDestroyed = true;
  // Recursively delete all children.
  for(size_t i = 0; i < numChildren(); i++)
    delete child(i);

  // Delete the dot entry.
  if (_dotEntry) {
    delete _dotEntry;
  }
}

void KDirInfo::recalcOneChild(KFileInfo * child) {
  _totalSize += child->totalSize();
  _totalBlocks += child->totalBlocks();
  _totalItems += child->totalItems() + 1;
  _totalSubDirs += child->totalSubDirs();
  _totalFiles += child->totalFiles();

  if (child->isDir())
    _totalSubDirs++;

  if (child->isFile())
    _totalFiles++;

  time_t childLatestMtime = child->latestMtime();

  if (childLatestMtime > _latestMtime)
    _latestMtime = childLatestMtime;
}

void KDirInfo::recalc() {
  // qDebug() << Q_FUNC_INFO << this << endl;

  _totalSize = _size;
  _totalBlocks = _blocks;
  _totalItems = 0;
  _totalSubDirs = 0;
  _totalFiles = 0;
  _latestMtime = _mtime;
  for(size_t i = 0; i < numChildren(); i++) {
    recalcOneChild(child(i));
  }
  if(dotEntry())
    recalcOneChild(dotEntry());
  _summaryDirty = false;
}

void KDirInfo::setMountPoint(bool isMountPoint) {
  _isMountPoint = isMountPoint;
}

KFileSize KDirInfo::totalSize() {
  if (_summaryDirty)
    recalc();

  return _totalSize;
}

KFileSize KDirInfo::totalBlocks() {
  if (_summaryDirty)
    recalc();

  return _totalBlocks;
}

int KDirInfo::totalItems() {
  if (_summaryDirty)
    recalc();

  return _totalItems;
}

int KDirInfo::totalSubDirs() {
  if (_summaryDirty)
    recalc();

  return _totalSubDirs;
}

int KDirInfo::totalFiles() {
  if (_summaryDirty)
    recalc();

  return _totalFiles;
}

time_t KDirInfo::latestMtime() {
  if (_summaryDirty)
    recalc();

  return _latestMtime;
}

bool KDirInfo::isFinished() { return !isBusy(); }

void KDirInfo::setReadState(KDirReadState newReadState) {
  // "aborted" has higher priority than "finished"

  if (_readState == KDirAborted && newReadState == KDirFinished)
    return;

  _readState = newReadState;
}

bool KDirInfo::isBusy() {
  if (_pendingReadJobs > 0 && _readState != KDirAborted)
    return true;

  if (readState() == KDirReading || readState() == KDirQueued)
    return true;

  return false;
}

void KDirInfo::insertChild(KFileInfo *newChild) {
  Q_CHECK_PTR(newChild);

  if (newChild->isDir() || _dotEntry == 0 || _isDotEntry) {
    /**
     * Only directories are stored directly in pure directory nodes -
     * unless something went terribly wrong, e.g. there is no dot entry to use.
     * If this is a dot entry, store everything it gets directly within it.
     *
     * In any of those cases, insert the new child in the children list.
     *
     * We don't bother with this list's order - it's explicitly declared to
     * be unordered, so be warned! We simply insert this new child at the
     * list head since this operation can be performed in constant time
     * without the need for any additional lastChild etc. pointers or -
     * even worse - seeking the correct place for insertion first. This is
     * none of our business; the corresponding "view" object for this tree
     * will take care of such niceties.
     **/
    children_.push_back(newChild);
    newChild->setParent(this); // make sure the parent pointer is correct

    childAdded(newChild); // update summaries
  } else {
    /*
     * If the child is not a directory, don't store it directly here - use
     * this entry's dot entry instead.
     */
    _dotEntry->insertChild(newChild);
  }
}

void KDirInfo::childAdded(KFileInfo *newChild) {
  if (!_summaryDirty) {
    _totalSize += newChild->totalSize();
    _totalBlocks += newChild->blocks();
    _totalItems++;

    if (newChild->isDir())
      _totalSubDirs++;

    if (newChild->isFile())
      _totalFiles++;

    if (newChild->mtime() > _latestMtime)
      _latestMtime = newChild->mtime();
  } else {
    // NOP

    /*
     * Don't bother updating the summary fields if the summary is dirty
     * (i.e. outdated) anyway: As soon as anybody wants to know some exact
     * value a complete recalculation of the entire subtree will be
     * triggered. On the other hand, if nobody wants to know (which is very
     * likely) we can save this effort.
     */
  }

  if (_parent)
    _parent->childAdded(newChild);
}

void KDirInfo::deletingChild(KFileInfo *deletedChild) {
  /**
   * When children are deleted, things go downhill: Marking the summary
   * fields as dirty (i.e. outdated) is the only thing that can be done here.
   *
   * The accumulated sizes could be updated (by subtracting this deleted
   * child's values from them), but the latest mtime definitely has to be
   * recalculated: The child now being deleted might just be the one with the
   * latest mtime, and figuring out the second-latest cannot easily be
   * done. So we merely mark the summary as dirty and wait until a recalc()
   * will be triggered from outside - which might as well never happen when
   * nobody wants to know some summary field anyway.
   **/

  _summaryDirty = true;

  if (_parent)
    _parent->deletingChild(deletedChild);

  if (!_beingDestroyed && deletedChild->parent() == this) {
    /**
     * Unlink the child from the children's list - but only if this doesn't
     * happen recursively in the destructor of this object: No use
     * bothering about the validity of the children's list if this will all
     * be history anyway in a moment.
     **/
    if (deletedChild->parent() != this) {
      qCritical() << deletedChild << " is not a child of " << this
                  << " - cannot unlink from children list!" << Qt::endl;
      return;
    }
    auto it = std::find(children_.begin(), children_.end(), deletedChild);
    if(it == children_.end()) {
      qCritical() << "Couldn't unlink " << deletedChild << " from " << this
                  << " children list" << Qt::endl;
    } else {
      children_.erase(it);
    }
  }
}

void KDirInfo::readJobAdded() {
  _pendingReadJobs++;

  if (_parent)
    _parent->readJobAdded();
}

void KDirInfo::readJobFinished() {
  _pendingReadJobs--;

  if (_parent)
    _parent->readJobFinished();
}

void KDirInfo::readJobAborted() {
  _readState = KDirAborted;

  if (_parent)
    _parent->readJobAborted();
}

void KDirInfo::finalizeLocal() { cleanupDotEntries(); }

void KDirInfo::finalizeAll(KDirTree* tree) {
  if (_isDotEntry)
    return;

  for(size_t i = 0; i < numChildren(); i++) {
    if(child(i)->isDirInfo()) {
      KDirInfo *dir = static_cast<KDirInfo *>(child(i));
      if(!dir->isDotEntry())
        dir->finalizeAll(tree);
    }
  }

  // Optimization: As long as this directory is not finalized yet, it does
  // (very likely) have a dot entry and thus all direct children are
  // subdirectories, not plain files, so we don't need to bother checking
  // plain file children as well - so do finalizeLocal() only after all
  // children are processed. If this step were the first, for directories
  // that don't have any subdirectories finalizeLocal() would immediately
  // get all their plain file children reparented to themselves, so they
  // would need to be processed in the loop, too.

  tree->sendFinalizeLocal(this); // Must be sent _before_ finalizeLocal()!
  finalizeLocal();
}

KDirReadState KDirInfo::readState() const {
  if (_isDotEntry && _parent)
    return _parent->readState();
  else
    return _readState;
}

void KDirInfo::cleanupDotEntries() {
  if (!_dotEntry || _isDotEntry) {
    children_.shrink_to_fit();
    return;
  }

  // Reparent dot entry children if there are no subdirectories on this level

  if (numChildren() == 0) {
    // qDebug() << "Reparenting children of solo dot entry " << this << endl;
    children_ = _dotEntry->children_;
    _dotEntry->children_.clear();
    for(size_t i = 0; i < numChildren(); i++)
      children_[i]->setParent(this);
  }

  // Delete dot entries without any children

  if (_dotEntry->numChildren() == 0) {
    // qDebug() << "Removing empty dot entry " << this << endl;

    delete _dotEntry;
    _dotEntry = 0;
  }
  if(_dotEntry)
    _dotEntry->cleanupDotEntries(); // just to shrink_to_fit
  children_.shrink_to_fit();
}
