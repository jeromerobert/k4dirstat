/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include "kdirreadjob.h"
#include "kdirtree.h"
#include "kdirtreecache.h"
#include <KSharedConfig>
#include <QDir>
#include <kconfig.h>
#include <kconfiggroup.h>
using namespace KDirStat;

KDirTree::KDirTree() : QObject() {
  _root = 0;
  _isFileProtocol = false;
  _isBusy = false;
  _readMethod = KDirReadUnknown;

  readConfig();

  connect(&_jobQueue, SIGNAL(finished()), this, SLOT(slotFinished()));
}

KDirTree::~KDirTree() {
  _jobQueue.clear();
  selectItems();

  if (_root)
    delete _root;
}

void KDirTree::readConfig() {
  KConfigGroup config = KSharedConfig::openConfig()->group("Directory Reading");

  _crossFileSystems = config.readEntry("CrossFileSystems", false);
  _enableLocalDirReader = config.readEntry("EnableLocalDirReader", true);
}

void KDirTree::setRoot(KFileInfo *newRoot) {
  if (_root) {
    selectItems();
    emit deletingChild(_root);
    delete _root;
    emit childDeleted();
  }

  _root = newRoot;
}

void KDirTree::clear(bool sendSignals) {
  _jobQueue.clear();

  if (_root) {
    selectItems();

    if (sendSignals)
      emit deletingChild(_root);

    delete _root;
    _root = 0;

    if (sendSignals)
      emit childDeleted();
  }

  _isBusy = false;
}

void KDirTree::startReading(const QUrl &url) {
  // qDebug() << Q_FUNC_INFO << " " << url.url() << endl;

#if 0
    qDebug() << "url: "		<< url.url()		<< endl;
    qDebug() << "path: "		<< url.path()		<< endl;
    qDebug() << "filename: "		<< url.filename() 	<< endl;
    qDebug() << "protocol: "		<< url.protocol() 	<< endl;
    qDebug() << "isValid: "		<< url.isValid() 	<< endl;
    qDebug() << "isMalformed: "	<< url.isMalformed() 	<< endl;
    qDebug() << "isLocalFile: "	<< url.isLocalFile() 	<< endl;
#endif

  _isBusy = true;
  emit startingReading();

  setRoot(0);
  readConfig();
  _isFileProtocol = url.isLocalFile();

  if (_isFileProtocol && _enableLocalDirReader) {
    // qDebug() << "Using local directory reader for " << url.url() << endl;
    _readMethod = KDirReadLocal;
    _root = KLocalDirReadJob::stat(url);
  } else {
    // qDebug() << "Using KIO methods for " << url.url() << endl;
    _readMethod = KDirReadKIO;
    _root = KioDirReadJob::stat(url);
  }

  if (_root) {
    childAddedNotify(_root);

    if (_root->isDir()) {
      KDirInfo *dir = (KDirInfo *)_root;

      if (_readMethod == KDirReadLocal)
        addJob(new KLocalDirReadJob(this, dir));
      else
        addJob(new KioDirReadJob(this, dir));
    } else {
      _isBusy = false;
      emit finished();
    }
  } else // stat() failed
  {
    // qWarning() << "stat(" << url.url() << ") failed" << endl;
    _isBusy = false;
    emit finished();
    emit finalizeLocal(0);
  }
}

void KDirTree::selectionInSubTree(KFileInfo *subtree) {
  for(size_t i = 0; i < _selection.size(); i++) {
    if(_selection[i]->isInSubtree(subtree)) {
      selectItems();
      break;
    }
  }
}

void KDirTree::refresh(KFileInfo *subtree) {
  if (!_root)
    return;

  if (!subtree || !subtree->parent()) // Refresh all (from root)
  {
    startReading(fixedUrl(_root->url()));
  } else // Refresh subtree
  {
    // Save some values from the old subtree.

    QUrl url = QUrl::fromUserInput(subtree->url(), QDir::currentPath(),
                                   QUrl::AssumeLocalFile);
    KDirInfo *parent = subtree->parent();

    // Select nothing if the current selection is to be deleted
    selectionInSubTree(subtree);

    // Clear any old "excluded" status

    subtree->setExcluded(false);

    // Get rid of the old subtree.

    emit deletingChild(subtree);

    // qDebug() << "Deleting subtree " << subtree << endl;

    /**
     * This may sound stupid, but the parent must be told to unlink its
     * child from the children list. The child cannot simply do this by
     * itself in its destructor since at this point important parts of the
     * object may already be destroyed, e.g., the virtual table -
     * i.e. virtual methods won't work any more.
     *
     * I just found that out the hard way by several hours of debugging. ;-}
     **/
    parent->deletingChild(subtree);
    delete subtree;
    emit childDeleted();

    _isBusy = true;
    emit startingReading();

    // Create new subtree root.

    subtree = (_readMethod == KDirReadLocal)
                  ? KLocalDirReadJob::stat(url, parent)
                  : KioDirReadJob::stat(url, parent);

    // qDebug() << "New subtree: " << subtree << endl;

    if (subtree) {
      // Insert new subtree root into the tree hierarchy.

      parent->insertChild(subtree);
      childAddedNotify(subtree);

      if (subtree->isDir()) {
        // Prepare reading this subtree's contents.

        KDirInfo *dir = (KDirInfo *)subtree;

        if (_readMethod == KDirReadLocal)
          addJob(new KLocalDirReadJob(this, dir));
        else
          addJob(new KioDirReadJob(this, dir));
      } else {
        _isBusy = false;
        emit finished();
      }
    }
  }
}

void KDirTree::abortReading() {
  if (_jobQueue.isEmpty())
    return;

  _jobQueue.abort();

  _isBusy = false;
  emit aborted();
}

void KDirTree::slotFinished() {
  _isBusy = false;
  emit finished();
}

void KDirTree::childAddedNotify(KFileInfo *newChild) {
  emit childAdded(newChild);

  if (newChild->dotEntry())
    emit childAdded(newChild->dotEntry());
}

void KDirTree::deletingChildNotify(KFileInfo *deletedChild) {
  emit deletingChild(deletedChild);

  // Only now check for selection and root: Give connected objects
  // (i.e. views) a chance to change either while handling the signal.
  selectionInSubTree(deletedChild);

  if (deletedChild == _root)
    _root = 0;
}

void KDirTree::childDeletedNotify() { emit childDeleted(); }

void KDirTree::deleteSubtree(KFileInfo *subtree) {
  // qDebug() << "Deleting subtree " << subtree << endl;
  KDirInfo *parent = subtree->parent();

  if (parent) {
    // Give the parent of the child to be deleted a chance to unlink the
    // child from its children list and take care of internal summary
    // fields
    parent->deletingChild(subtree);
  }

  // Send notification to anybody interested (e.g., to attached views)
  deletingChildNotify(subtree);

  if (parent) {
    if (parent->isDotEntry() && !parent->hasChildren())
    // This was the last child of a dot entry
    {
      // Get rid of that now empty and useless dot entry

      if (parent->parent()) {
        if (parent->parent()->isFinished()) {
          // qDebug() << "Removing empty dot entry " << parent << endl;

          deletingChildNotify(parent);
          parent->parent()->setDotEntry(0);

          delete parent;
        }
      } else // no parent - this should never happen (?)
      {
        qCritical() << "Internal error: Killing dot entry without parent "
                    << parent << Qt::endl;

        // Better leave that dot entry alone - we shouldn't have come
        // here in the first place. Who knows what will happen if this
        // thing is deleted now?!
        //
        // Intentionally NOT calling:
        //     delete parent;
      }
    }
  }

  delete subtree;

  if (subtree == _root) {
    selectItems();
    _root = 0;
  }

  emit childDeleted();
}

void KDirTree::addJob(KDirReadJob *job) { _jobQueue.enqueue(job); }

void KDirTree::sendProgressInfo(const QString &infoLine) {
  emit progressInfo(infoLine);
}

void KDirTree::sendFinalizeLocal(KDirInfo *dir) { emit finalizeLocal(dir); }

void KDirTree::sendStartingReading() { emit startingReading(); }

void KDirTree::sendFinished() { emit finished(); }

void KDirTree::sendAborted() { emit aborted(); }

void KDirTree::selectItems(const std::vector<KFileInfo *> & newSelection) {
  if (newSelection != _selection) {
    _selection = newSelection;
    emit selectionChanged(this);
  }
}

bool KDirTree::writeCache(const QString &cacheFileName) {
  KCacheWriter writer(cacheFileName, this);
  return writer.ok();
}

void KDirTree::readCache(const QString &cacheFileName) {
  _isBusy = true;
  emit startingReading();
  addJob(new KCacheReadJob(this, 0, cacheFileName));
}

