/*
 *   File name:	kstdcleanup.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 *
 *   Updated:	2010-03-14
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "k4dirstat.h"
#include "kcleanup.h"
#include "kstdcleanup.h"
#include <KIO/CopyJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KJobWidgets>
#include <KLocalizedString>

using namespace KDirStat;

KCleanup *KStdCleanup::openInKonqueror(QString &icon, QKeySequence &shortcut) {
  KCleanup *cleanup = new KCleanup("cleanup_open_in_konqueror", "xdg-open %p",
                                   i18n("Open a file &browser"));
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(true);
  cleanup->setWorksForDotEntry(true);
  cleanup->setWorksLocalOnly(false);
  cleanup->setRefreshPolicy(KCleanup::noRefresh);
  icon = "konqueror";
  shortcut = Qt::CTRL + Qt::Key_K;
  return cleanup;
}

KCleanup *KStdCleanup::openInTerminal(QString &icon, QKeySequence &shortcut) {
  KCleanup *cleanup = new KCleanup("cleanup_open_in_terminal", "konsole",
                                   i18n("Open in &Terminal"));
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(true);
  cleanup->setWorksForDotEntry(true);
  cleanup->setRefreshPolicy(KCleanup::noRefresh);
  icon = "utilities-terminal";
  shortcut = Qt::CTRL + Qt::Key_T;

  return cleanup;
}

KCleanup *KStdCleanup::compressSubtree(QString &iconFile, QKeySequence &) {
  KCleanup *cleanup = new KCleanup("cleanup_compress_subtree",
                                   "cd ..; tar cjvf %n.tar.bz2 %n && rm -rf %n",
                                   i18n("&Compress"));
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(false);
  cleanup->setWorksForDotEntry(false);
  cleanup->setRefreshPolicy(KCleanup::refreshParent);
  iconFile = "utilities-file-archiver";

  return cleanup;
}

KCleanup *KStdCleanup::makeClean(QString &, QKeySequence &) {
  KCleanup *cleanup =
      new KCleanup("cleanup_make_clean", "make clean", i18n("&make clean"));
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(false);
  cleanup->setWorksForDotEntry(true);
  cleanup->setRefreshPolicy(KCleanup::refreshThis);

  return cleanup;
}

KCleanup *KStdCleanup::deleteTrash(QString &, QKeySequence &) {
  KCleanup *cleanup =
      new KCleanup("cleanup_delete_trash", "rm -f *.o *~ *.bak *.auto core",
                   i18n("Delete T&rash Files"));
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(false);
  cleanup->setWorksForDotEntry(true);
  cleanup->setRefreshPolicy(KCleanup::refreshThis);
  cleanup->setRecurse(true);

  return cleanup;
}

KCleanup *KStdCleanup::moveToTrashBin(QString &icon, QKeySequence &shortcut) {
  KCleanup *cleanup = new TrashBinCleanup();
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(true);
  cleanup->setWorksForDotEntry(false);
  cleanup->setRefreshPolicy(KCleanup::assumeDeleted);
  /* The icon standard says the action should be "edit-trash"
     However, Oxygen doesn't have that icon, so I'm setting
     "user-trash" which will probably be the same in most
     icon sets. */
  // cleanup->setIcon(KIcon( "edit-trash" ));
  icon = "user-trash";
  shortcut = Qt::CTRL + Qt::Key_X;

  return cleanup;
}

KCleanup *KStdCleanup::hardDelete(QString &icon, QKeySequence &shortcut) {
  KCleanup *cleanup = new KCleanup("cleanup_hard_delete", "rm -rf %p",
                                   i18n("&Delete (no way to undelete!)"));
  Q_CHECK_PTR(cleanup);
  cleanup->setWorksForDir(true);
  cleanup->setWorksForFile(true);
  cleanup->setWorksForDotEntry(false);
  cleanup->setAskForConfirmation(true);
  cleanup->setRefreshPolicy(KCleanup::assumeDeleted);
  icon = "edit-delete";
  shortcut = Qt::CTRL + Qt::Key_Delete;

  return cleanup;
}

TrashBinCleanup::TrashBinCleanup()
    : KCleanup("cleanup_move_to_trash_bin", "",
               i18n("Delete (to Trash &Bin)")) {}

static void konqOperationsDel(QWidget *m_mainWindow, const QList<QUrl> &urls) {
  KIO::JobUiDelegate uiDelegate;
  uiDelegate.setWindow(m_mainWindow);
  if (uiDelegate.askDeleteConfirmation(
          urls, KIO::JobUiDelegate::Trash,
          KIO::JobUiDelegate::DefaultConfirmation)) {
    KIO::Job *job = KIO::trash(urls);
    KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, urls,
                                            QUrl("trash:/"), job);
    KJobWidgets::setWindow(job, m_mainWindow);
    job->uiDelegate()->setAutoErrorHandlingEnabled(
        true); // or connect to the result signal
  }
}

void TrashBinCleanup::execute(KFileInfo *item) {
  if (worksFor(item)) {
    QUrl url;
    url.setPath(item->url());
    QList<QUrl> urls;
    urls.append(url);
    konqOperationsDel(k4dirstat::instance(), urls);
    item->tree()->deleteSubtree(item);
  }
}

// EOF
