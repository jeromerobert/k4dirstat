/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include "kcleanupcollection.h"
#include "kcleanupcollection_p.h"
#include "kstdcleanup.h"
#include <KActionCollection>
#include <KIconEngine>
#include <KIconLoader>
#include <KLocalizedString>
#include <QIcon>

using namespace KDirStat;

void CleanupAction::selectionChanged(KDirTree * tree) {
  tree_ = tree;
  setEnabled(cleanup_.isEnabledFromSelection(tree));
}

KCleanupCollection::KCleanupCollection(KActionCollection &actionCollection)
    : _actionCollection(actionCollection) {
  /**
   * All cleanups beloningt to this collection are stored in two separate Qt
   * collections, a QList and a QDict. Make _one_ of them manage the cleanup
   * objects, i.e. have them clear the KCleanup objects upon deleting. The
   * QList is the master collection, the QDict the slave.
   **/

  _nextUserCleanupNo = 0;
}

KCleanupCollection::~KCleanupCollection() {
  // No need to delete the cleanups: _cleanupList takes care of that
  // (autoDelete!).
}

CleanupAction *KCleanupCollection::add(KCleanup *newCleanup) {
  CleanupAction *action = new CleanupAction(*newCleanup, this);
  _actionCollection.addAction(newCleanup->id(), action);
  delete newCleanup;
  cleanupActions.append(action);

  connect(this, SIGNAL(selectionChanged(KDirTree*)), action,
          SLOT(selectionChanged(KDirTree*)));

  connect(action, SIGNAL(executed()), this, SLOT(cleanupExecuted()));
  return action;
}

void KCleanupCollection::readConfig() {
  for (int i = 0; i < cleanupActions.count(); i++) {
    cleanupActions[i]->cleanup().readConfig();
    cleanupActions[i]->refresh();
  }
}

void KCleanupCollection::saveConfig() {
  for (int i = 0; i < cleanupActions.count(); i++) {
    cleanupActions[i]->cleanup().saveConfig();
  }
}

void KCleanupCollection::add(KCleanup *(*factory)(QString &iconName,
                                                  QKeySequence &shortcut)) {
  QString iconName;
  QKeySequence shortcut;
  QAction *action;
  action = add(factory(iconName, shortcut));
  if (!shortcut.isEmpty())
    _actionCollection.setDefaultShortcut(action, shortcut);
  if (!iconName.isEmpty())
    action->setIcon(QIcon(new KIconEngine(iconName, KIconLoader::global())));
}

void KCleanupCollection::addStdCleanups() {
  add(KStdCleanup::openInKonqueror);
  add(KStdCleanup::openInTerminal);
  add(KStdCleanup::compressSubtree);
  add(KStdCleanup::makeClean);
  add(KStdCleanup::deleteTrash);
  add(KStdCleanup::moveToTrashBin);
  add(KStdCleanup::hardDelete);
}

void KCleanupCollection::addUserCleanups(int number) {
  for (int i = 0; i < number; i++) {
    QString id = QString("cleanup_user_defined_%1").arg(_nextUserCleanupNo);
    QString title;

    if (_nextUserCleanupNo <= 9)
      // Provide a keyboard shortcut for cleanup #0..#9
      title = i18n("User Defined Cleanup #&%1", _nextUserCleanupNo);
    else
      // No keyboard shortcuts for cleanups #10.. - they would be duplicates
      title = i18n("User Defined Cleanup #%1", _nextUserCleanupNo);

    _nextUserCleanupNo++;

    KCleanup *cleanup = new KCleanup(id, "", title);
    Q_CHECK_PTR(cleanup);
    cleanup->setEnabled(false);

    CleanupAction *action = add(cleanup);
    if (i <= 9) {
      // Provide an application-wide keyboard accelerator for cleanup #0..#9
      _actionCollection.setDefaultShortcut(action, Qt::CTRL + Qt::Key_0 + i);
    }
  }
}

void KCleanupCollection::cleanupExecuted() { emit userActivity(10); }

void KCleanupCollection::revertToDefault(int nbUserCleanups) {
  _nextUserCleanupNo = 0;
  for (int i = 0; i < cleanupActions.size(); i++)
    _actionCollection.removeAction(cleanupActions[i]);
  cleanupActions.clear();
  addStdCleanups();
  addUserCleanups(nbUserCleanups);
}

QList<KCleanup> KCleanupCollection::cleanupsCopy() {
  QList<KCleanup> toReturn;
  toReturn.reserve(cleanupActions.size());
  for (int i = 0; i < cleanupActions.size(); i++)
    toReturn.append(cleanupActions[i]->cleanup());
  return toReturn;
}

void KCleanupCollection::setCleanups(QList<KCleanup> &cleanups) {
  Q_ASSERT(cleanups.size() == cleanupActions.size());
  for (int i = 0; i < cleanups.size(); i++) {
    cleanupActions[i]->cleanup() = cleanups[i];
    cleanupActions[i]->refresh();
  }
}

