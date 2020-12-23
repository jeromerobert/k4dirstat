/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <kio/job.h>
#include <stdio.h>
#include <sys/errno.h>

#include "k4dirstat.h"
#include "kdirreadjob.h"
#include "kdirtree.h"
#include "kdirtreecache.h"
#include "kexcluderules.h"
#include <QDir>

using namespace KDirStat;

KDirReadJob::KDirReadJob(KDirTree *tree, KDirInfo *dir)
    : _tree(tree), _dir(dir) {
  _queue = 0;
  _started = false;

  if (_dir)
    _dir->readJobAdded();
}

KDirReadJob::~KDirReadJob() {
  if (_dir)
    _dir->readJobFinished();
}

/**
 * Default implementation - derived classes should overwrite this method or
 * startReading() (or both).
 **/

void KDirReadJob::read() {
  if (!_started) {
    _started = true;
    startReading();

    // Don't do anything after startReading() - startReading() might call
    // finished() which in turn makes the queue destroy this object
  }
}

void KDirReadJob::setDir(KDirInfo *dir) { _dir = dir; }

void KDirReadJob::finished() {
  if (_queue)
    _queue->jobFinishedNotify(this);
  else
    qCritical() << "No job queue for " << _dir << Qt::endl;
}

void KDirReadJob::childAdded(KFileInfo *newChild) {
  _tree->childAddedNotify(newChild);
}

void KDirReadJob::deletingChild(KFileInfo *deletedChild) {
  _tree->deletingChildNotify(deletedChild);
}

KLocalDirReadJob::KLocalDirReadJob(KDirTree *tree, KDirInfo *dir)
    : KDirReadJob(tree, dir), _diskDir(0) {}

KLocalDirReadJob::~KLocalDirReadJob() {}

void KLocalDirReadJob::startReading() {
  struct dirent *entry;
  struct stat statInfo;
  QString dirName = _dir->url();

  if ((_diskDir = opendir(dirName.toLocal8Bit()))) {
    _tree->sendProgressInfo(dirName);
    _dir->setReadState(KDirReading);

    while ((entry = readdir(_diskDir))) {
      QString entryName = entry->d_name;

      if (entryName != "." && entryName != "..") {
        QString fullName = dirName + "/" + entryName;
        QByteArray rawFullName = dirName.toLocal8Bit() +
                                 QDir::separator().toLatin1() +
                                 QByteArray(entry->d_name);
        if (lstat(rawFullName.data(), &statInfo) == 0) // lstat() OK
        {
          if (S_ISDIR(statInfo.st_mode)) // directory child?
          {
            KDirInfo *subDir = new KDirInfo(entryName, &statInfo, _dir);
            _dir->insertChild(subDir);
            childAdded(subDir);

            if (KExcludeRules::excludeRules()->match(fullName)) {
              subDir->setExcluded();
              subDir->setReadState(KDirOnRequestOnly);
              _tree->sendFinalizeLocal(subDir);
              subDir->finalizeLocal();
            } else // No exclude rule matched
            {
              if (_dir->device() == subDir->device()) // normal case
              {
                _tree->addJob(new KLocalDirReadJob(_tree, subDir));
              } else // The subdirectory we just found is a mount point.
              {
                // qDebug() << "Found mount point " << subDir << endl;
                subDir->setMountPoint();

                if (_tree->crossFileSystems()) {
                  _tree->addJob(new KLocalDirReadJob(_tree, subDir));
                } else {
                  subDir->setReadState(KDirOnRequestOnly);
                  _tree->sendFinalizeLocal(subDir);
                  subDir->finalizeLocal();
                }
              }
            }
          } else // non-directory child
          {
            if (entryName == DEFAULT_CACHE_NAME) // .kdirstat.cache.gz found?
            {
              //
              // Read content of this subdirectory from cache file
              //

              KCacheReadJob *cacheReadJob =
                  new KCacheReadJob(_tree, _dir->parent(), fullName);
              Q_CHECK_PTR(cacheReadJob);
              QString firstDirInCache = cacheReadJob->reader()->firstDir();

              if (firstDirInCache ==
                  dirName) // Does this cache file match this directory?
              {
                qDebug() << "Using cache file " << fullName << " for "
                         << dirName << Qt::endl;

                cacheReadJob->reader()
                    ->rewind(); // Read offset was moved by firstDir()
                _tree->addJob(cacheReadJob); // Job queue will assume ownership
                                             // of cacheReadJob

                //
                // Clean up partially read directory content
                //

                KDirTree *tree = _tree; // Copy data members to local variables:
                KDirInfo *dir =
                    _dir; // This object will be deleted soon by killAll()

                _queue->killAll(dir); // Will delete this job as well!
                // All data members of this object are invalid from here on!

                tree->deleteSubtree(dir);

                return;
              } else {
                qDebug() << "NOT using cache file " << fullName << " with dir "
                         << firstDirInCache << " for " << dirName << Qt::endl;

                delete cacheReadJob;
              }
            } else {
              KFileInfo *child = new KFileInfo(entryName, &statInfo, _dir);
              _dir->insertChild(child);
              childAdded(child);
            }
          }
        } else // lstat() error
        {
          qWarning() << "lstat(" << fullName << ") failed: " << strerror(errno)
                     << Qt::endl;

          /*
           * Not much we can do when lstat() didn't work; let's at
           * least create an (almost empty) entry as a placeholder.
           */
          KDirInfo *child = new KDirInfo(_dir, entry->d_name);
          child->setReadState(KDirError);
          _dir->insertChild(child);
          childAdded(child);
        }
      }
    }

    closedir(_diskDir);
    // qDebug() << "Finished reading " << _dir << endl;
    _dir->setReadState(KDirFinished);
    _dir->finalizeLocal();
    _tree->sendFinalizeLocal(_dir);
  } else {
    _dir->setReadState(KDirError);
    _dir->finalizeLocal();
    _tree->sendFinalizeLocal(_dir);
    // qWarning() << Q_FUNC_INFO << "opendir(" << dirName << ") failed" << endl;
    // opendir() doesn't set 'errno' according to POSIX  :-(
  }

  finished();
  // Don't add anything after finished() since this deletes this job!
}

KFileInfo *KLocalDirReadJob::stat(const QUrl &url, KDirInfo *parent) {
  struct stat statInfo;

  if (lstat(url.path().toLocal8Bit(), &statInfo) == 0) // lstat() OK
  {
    QString name = parent ? url.fileName() : url.path();

    if (S_ISDIR(statInfo.st_mode)) // directory?
    {
      KDirInfo *dir = new KDirInfo(name, &statInfo, parent);

      if (dir && parent && dir->device() != parent->device())
        dir->setMountPoint();

      return dir;
    } else // no directory
      return new KFileInfo(name, &statInfo, parent);
  } else // lstat() failed
    return 0;
}

KioDirReadJob::KioDirReadJob(KDirTree *tree, KDirInfo *dir)
    : KObjDirReadJob(tree, dir) {
  _job = 0;
}

KioDirReadJob::~KioDirReadJob() {
#if 0
    if ( _job )
	_job->kill( true );	// quietly
#endif
}

void KioDirReadJob::startReading() {
  QUrl url = QUrl::fromUserInput(_dir->url(), QDir::currentPath(),
                                 QUrl::AssumeLocalFile);
  if (!url.isValid()) {
    qWarning() << Q_FUNC_INFO << "URL malformed: " << _dir->url() << Qt::endl;
  }

  _job = KIO::listDir(url);

  connect(_job, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)), this,
          SLOT(entries(KIO::Job *, const KIO::UDSEntryList &)));

  connect(_job, SIGNAL(result(KIO::Job *)), this, SLOT(finished(KIO::Job *)));

  connect(_job, SIGNAL(canceled(KIO::Job *)), this, SLOT(finished(KIO::Job *)));
}

void KioDirReadJob::entries(KIO::Job *job, const KIO::UDSEntryList &entryList) {
  NOT_USED(job);
  QUrl url(_dir->url()); // Cache this - it's expensive!

  if (!url.isValid()) {
    qWarning() << Q_FUNC_INFO << "URL malformed: " << _dir->url() << Qt::endl;
  }

  KIO::UDSEntryList::ConstIterator it = entryList.begin();

  while (it != entryList.end()) {
    KFileItem entry(*it, url,
                    true,  // determineMimeTypeOnDemand
                    true); // URL is parent directory

    if (entry.name() != "." && entry.name() != "..") {
      // qDebug() << "Found " << entry.url().url() << endl;

      if (entry.isDir() && // Directory child
          !entry.isLink()) // and not a symlink?
      {
        KDirInfo *subDir = new KDirInfo(&entry, _dir);
        _dir->insertChild(subDir);
        childAdded(subDir);

        if (KExcludeRules::excludeRules()->match(url.path())) {
          subDir->setExcluded();
          subDir->setReadState(KDirOnRequestOnly);
          _tree->sendFinalizeLocal(subDir);
          subDir->finalizeLocal();
        } else // No exclude rule matched
        {
          _tree->addJob(new KioDirReadJob(_tree, subDir));
        }
      } else // non-directory child
      {
        KFileInfo *child = new KFileInfo(&entry, _dir);
        _dir->insertChild(child);
        childAdded(child);
      }
    }

    ++it;
  }
}

void KioDirReadJob::finished(KIO::Job *job) {
  if (job->error())
    _dir->setReadState(KDirError);
  else
    _dir->setReadState(KDirFinished);

  _tree->sendFinalizeLocal(_dir);
  _dir->finalizeLocal();
  _job = 0; // The job deletes itself after this signal!

  KDirReadJob::finished();
  // Don't add anything after finished() since this deletes this job!
}

KFileInfo *KioDirReadJob::stat(const QUrl &url, KDirInfo *parent) {
  KIO::StatJob *job = KIO::stat(url);
  if (job->exec()) {
    KFileItem entry(job->statResult(), url,
                    true,   // determine MIME type on demand
                    false); // URL specifies parent directory

    return entry.isDir() ? new KDirInfo(&entry, parent)
                         : new KFileInfo(&entry, parent);
  } else // remote stat() failed
    return 0;
}

QString KioDirReadJob::owner(QUrl url) {
  KIO::StatJob *job = KIO::stat(url);
  if (job->exec()) {
    KFileItem entry(job->statResult(), url,
                    true,   // determine MIME type on demand
                    false); // URL specifies parent directory

    return entry.user();
  }

  return QString();
}

KCacheReadJob::KCacheReadJob(KDirTree *tree, KDirInfo *parent,
                             KCacheReader *reader)
    : KObjDirReadJob(tree, parent), _reader(reader) {
  if (_reader)
    _reader->rewind();

  init();
}

KCacheReadJob::KCacheReadJob(KDirTree *tree, KDirInfo *parent,
                             const QString &cacheFileName)
    : KObjDirReadJob(tree, parent) {
  _reader = new KCacheReader(cacheFileName, tree, parent);
  Q_CHECK_PTR(_reader);

  init();
}

void KCacheReadJob::init() {
  if (_reader) {
    if (_reader->ok()) {
      connect(_reader, SIGNAL(childAdded(KFileInfo *)), this,
              SLOT(slotChildAdded(KFileInfo *)));
    } else {
      delete _reader;
      _reader = 0;
    }
  }
}

KCacheReadJob::~KCacheReadJob() {
  if (_reader)
    delete _reader;
}

void KCacheReadJob::read() {
  /*
   * This will be called repeatedly from KDirTree::timeSlicedRead() until
   * finished() is called.
   */

  if (!_reader) {
    finished();
    return;
  }

  // qDebug() << "Reading 1000 cache lines" << endl;
  _reader->read(1000);
  _tree->sendProgressInfo("");

  if (_reader->eof() || !_reader->ok()) {
    // qDebug() << "Cache reading finished - ok: " << _reader->ok() << endl;
    finished();
  }
}

KDirReadJobQueue::KDirReadJobQueue() : QObject() {

  connect(&_timer, SIGNAL(timeout()), this, SLOT(timeSlicedRead()));
}

KDirReadJobQueue::~KDirReadJobQueue() { clear(); }

void KDirReadJobQueue::enqueue(KDirReadJob *job) {
  if (job) {
    _queue.append(job);
    job->setQueue(this);

    if (!_timer.isActive()) {
      // qDebug() << "First job queued" << endl;
      emit startingReading();
      _timer.start(0);
    }
  }
}

KDirReadJob *KDirReadJobQueue::dequeue() {
  KDirReadJob *job = _queue.first();
  _queue.removeFirst();

  if (job)
    job->setQueue(0);

  return job;
}

void KDirReadJobQueue::clear() {
  QMutableListIterator<KDirReadJob *> i(_queue);
  while (i.hasNext()) {
    KDirReadJob *job = i.next();
    delete job;
    i.remove();
  }
}

void KDirReadJobQueue::abort() {
  while (!_queue.isEmpty()) {
    KDirReadJob *job = _queue.first();

    if (job->dir())
      job->dir()->readJobAborted();

    _queue.removeFirst();
    delete job;
  }
}

void KDirReadJobQueue::killAll(KDirInfo *subtree) {
  if (!subtree)
    return;

  QMutableListIterator<KDirReadJob *> i(_queue);
  while (i.hasNext()) {
    KDirReadJob *job = i.next();
    if (job->dir() && job->dir()->isInSubtree(subtree)) {
      i.remove();
      delete job;
    } else {
    }
  }
}

void KDirReadJobQueue::timeSlicedRead() {
  if (!_queue.isEmpty())
    _queue.first()->read();
}

void KDirReadJobQueue::jobFinishedNotify(KDirReadJob *job) {
  // Get rid of the old (finished) job.

  _queue.removeFirst();
  delete job;

  // Look for a new job.

  if (_queue.isEmpty()) // No new job available - we're done.
  {
    _timer.stop();
    // qDebug() << "No more jobs - finishing" << endl;
    emit finished();
  }
}

