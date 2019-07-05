/*
 *   File name:	kcleanupcollection.h
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 *
 *   Updated:	2010-02-01
 */

#ifndef KCleanupCollection_h
#define KCleanupCollection_h

#include "kcleanup.h"

class KActionCollection;

namespace KDirStat {
class CleanupAction;
/**
 * Set of @ref KCleanup actions to be performed for @ref KDirTree items,
 * consisting of a number of predefined and a number of user-defined
 * cleanups. The prime purpose of this is to make save/restore operations
 * with a number of cleanups easier. Thus, it provides a copy constructor,
 * an assignment operator and various methods to directly access individual
 * cleanups.
 *
 * @short KDirStat cleanup action collection
 **/

class KCleanupCollection : public QObject {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * Most applications will want to pass KMainWindow::actionCollection()
   * for 'actionCollection' so the menus and toolbars can be created
   * using the XML UI description ('kdirstatui.rc' for KDirStat).
   *
   * All @ref KCleanup actions ever added to this collection will get
   * this as their parent.
   **/
  KCleanupCollection(KActionCollection &actionCollection);

  /**
   * Destructor
   **/
  virtual ~KCleanupCollection();

  /**
   * Add the standard cleanups to this collection.
   **/
  void addStdCleanups();

  /**
   * Add 'number' user-defined cleanups to this collection.
   **/
  void addUserCleanups(int number);

  QList<KCleanup> cleanupsCopy();
  void setCleanups(QList<KCleanup> &cleanups);
  void revertToDefault(int nbUserCleanups);

private:
  /**
   * Add one single cleanup to this collection. The collection assumes
   * ownerwhip of this cleanup - don't delete it!
   **/
  CleanupAction *add(KCleanup *cleanup);
  void add(KCleanup *(*factory)(QString &icon, QKeySequence &shortcut));

public slots:

  /**
   * Read collection for all cleanups.
   **/
  void readConfig();

  /**
   * Save configuration for all cleanups.
   **/
  void saveConfig();

signals:
  /**
   * Emitted at user activity, i.e. when the user executes a cleanup.
   * This is intended for use together with a @ref KActivityTracker.
   **/
  void userActivity(int points);
  void selectionChanged(KFileInfo *, KDirTree *);

protected slots:

  /**
   * Connected to each cleanup's @ref executed() signal to track user
   * activity.
   **/
  void cleanupExecuted();

private:
  // Data members

  KActionCollection &_actionCollection;
  QList<CleanupAction *> cleanupActions;
  int _nextUserCleanupNo;
};
} // namespace KDirStat

#endif // ifndef KCleanupCollection_h
