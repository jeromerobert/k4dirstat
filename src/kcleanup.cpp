/*
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include <qapplication.h>
#include <qregexp.h>
#include <stdlib.h>

#include <KSharedConfig>
#include <QProcess>
#include <kconfiggroup.h>
#include <kmessagebox.h>

#include "kcleanup.h"
#include <KLocalizedString>

#define VERBOSE_RUN_COMMAND 1
#define SIMULATE_COMMAND 0

using namespace KDirStat;

KCleanup::KCleanup(QString id, QString command, QString title)
    : _id(id), _command(command), _title(title) {
  _enabled = true;
  _worksForDir = true;
  _worksForFile = false;
  _worksForDotEntry = false;
  _worksLocalOnly = true;
  _recurse = false;
  _askForConfirmation = false;
  _refreshPolicy = noRefresh;
}

KCleanup::KCleanup(const KCleanup &src) { copy(src); }

KCleanup &KCleanup::operator=(const KCleanup &src) {
  copy(src);

  return *this;
}

void KCleanup::copy(const KCleanup &src) {
  setTitle(src.title());
  _id = src.id();
  _command = src.command();
  _enabled = src.enabled();
  _worksForDir = src.worksForDir();
  _worksForFile = src.worksForFile();
  _worksForDotEntry = src.worksForDotEntry();
  _worksLocalOnly = src.worksLocalOnly();
  _recurse = src.recurse();
  _askForConfirmation = src.askForConfirmation();
  _refreshPolicy = src.refreshPolicy();
}

void KCleanup::setTitle(const QString &title) { _title = title; }

bool KCleanup::worksFor(KFileInfo *item, KDirTree * tree) const {
  if (!_enabled || !item)
    return false;

  if (worksLocalOnly() && !tree->isFileProtocol())
    return false;

  if (item->isDotEntry())
    return worksForDotEntry();
  if (item->isDir())
    return worksForDir();

  return worksForFile();
}

bool KCleanup::isEnabledFromSelection(KFileInfo *selection, KDirTree* tree) {
  bool enabled = false;
  if (selection) {
    enabled = worksFor(selection, tree);

    if (!selection->isFinished()) {
      // This subtree isn't finished reading yet

      switch (_refreshPolicy) {
      // Refresh policies that would cause this subtree to be deleted
      case refreshThis:
      case refreshParent:
      case assumeDeleted:

        // Prevent premature deletion of this tree - this would
        // cause a core dump for sure.
        enabled = false;
        break;

      default:
        break;
      }
    }
  }
  return enabled;
}

bool KCleanup::confirmation(KFileInfo *item) {
  QString msg;

  if (item->isDir() || item->isDotEntry()) {
    msg = i18n("%1\nin directory %2", cleanTitle(), item->url());
  } else {
    msg = i18n("%1\nfor file %2", cleanTitle(), item->url());
  }

  if (KMessageBox::warningContinueCancel(
          0,                        // parentWidget
          msg,                      // message
          i18n("Please Confirm"),   // caption
          KGuiItem(i18n("Confirm")) // confirmButtonLabel
          ) == KMessageBox::Continue)
    return true;
  else
    return false;
}

void KCleanup::execute(KFileInfo *item, KDirTree * tree) {
  if (worksFor(item, tree)) {
    if (_askForConfirmation && !confirmation(item))
      return;
    executeRecursive(item, tree);

    switch (_refreshPolicy) {
    case noRefresh:
      // Do nothing.
      break;

    case refreshThis:
      tree->refresh(item);
      break;

    case refreshParent:
      tree->refresh(item->parent());
      break;

    case assumeDeleted:

      // Assume the cleanup action has deleted the item.
      // Modify the KDirTree accordingly.

      tree->deleteSubtree(item);

      // Don't try to figure out a reasonable next selection - the
      // views have to do that while handling the subtree
      // deletion. Only the views have any knowledge about a
      // reasonable strategy for choosing a next selection. Unlike
      // the view items, the KFileInfo items don't have an order that
      // makes any sense to the user.

      break;
    }
  }
}

void KCleanup::executeRecursive(KFileInfo *item, KDirTree* tree) {
  if (worksFor(item, tree)) {
    if (_recurse) {
      // Recurse into all subdirectories.

      KFileInfo *subdir = item->firstChild();

      while (subdir) {
        if (subdir->isDir()) {
          /**
           * Recursively execute in this subdirectory, but only if it
           * really is a directory: File children might have been
           * reparented to the directory (normally, they reside in
           * the dot entry) if there are no real subdirectories on
           * this directory level.
           **/
          executeRecursive(subdir, tree);
        }
        subdir = subdir->next();
      }
    }

    // Perform cleanup for this directory.

    runCommand(item, _command);
  }
}

const QString KCleanup::itemDir(const KFileInfo *item) const {
  QString dir = item->url();

  if (!item->isDir() && !item->isDotEntry()) {
    dir.replace(QRegExp("/[^/]*$"), "");
  }

  return dir;
}

QString KCleanup::cleanTitle() const {
  // Use the cleanup action's title, if possible.

  QString title = _title;

  if (title.isEmpty()) {
    title = _id;
  }

  // Get rid of any "&" characters in the text that denote keyboard
  // shortcuts in menus.
  title.replace(QRegExp("&"), "");

  return title;
}

QString KCleanup::expandVariables(const KFileInfo *item,
                                  const QString &unexpanded) const {
  QString expanded = unexpanded;
  QString url = item->url().replace("'", "'\\''");
  expanded.replace(QRegExp("%p"), "'" + url + "'");
  QString name = item->name().replace("'", "'\\''");
  expanded.replace(QRegExp("%n"), "'" + name + "'");

  // if ( KDE::versionMajor() >= 3 && KDE::versionMinor() >= 4 )
  expanded.replace(QRegExp("%t"), "trash:/");
  // else
  // expanded.replace( QRegExp( "%t" ), KGlobalSettings::trashPath() );

  return expanded;
}

#include <qtextcodec.h>
void KCleanup::runCommand(const KFileInfo *item, const QString &command) const {
  QProcess *proc = new QProcess();
  QStringList args;
  args << "-c" << expandVariables(item, command);

#if !SIMULATE_COMMAND
  switch (_refreshPolicy) {
  case noRefresh:
  case assumeDeleted:

    // In either case it is no use waiting for the command to
    // finish, so we are starting the command as a pure
    // background process.

    proc->start("sh", args);
    break;

  case refreshThis:
  case refreshParent:

    // If a display refresh is due after the command, we need to
    // wait for the command to be finished in order to avoid
    // performing the update prematurely, so we are starting this
    // process in blocking mode.

    QApplication::setOverrideCursor(Qt::WaitCursor);
    proc->start("sh", args);
    proc->waitForFinished();
    QApplication::restoreOverrideCursor();
    break;
  }

#endif
}

void KCleanup::readConfig() {
  KConfigGroup config = KSharedConfig::openConfig()->group(_id);

  bool valid = config.readEntry("valid", false);

  // If the config section requested exists, it should contain a
  // "valid" field with a true value. If not, there is no such
  // section within the config file. In this case, just leave this
  // cleanup action undisturbed - we'd rather have a good default
  // value (as provided - hopefully - by our application upon
  // startup) than a generic empty cleanup action.

  if (valid) {
    _command = config.readEntry("command");
    _enabled = config.readEntry("enabled", false);
    _worksForDir = config.readEntry("worksForDir", true);
    _worksForFile = config.readEntry("worksForFile", true);
    _worksForDotEntry = config.readEntry("worksForDotEntry", true);
    _worksLocalOnly = config.readEntry("worksLocalOnly", true);
    _recurse = config.readEntry("recurse", false);
    _askForConfirmation = config.readEntry("askForConfirmation", false);
    _refreshPolicy =
        (KCleanup::RefreshPolicy)config.readEntry("refreshPolicy", 0);
    setTitle(config.readEntry("title"));
  }
}

void KCleanup::saveConfig() const {
  KConfigGroup config = KSharedConfig::openConfig()->group(_id);

  config.writeEntry("valid", true);
  config.writeEntry("command", _command);
  config.writeEntry("title", _title);
  config.writeEntry("enabled", _enabled);
  config.writeEntry("worksForDir", _worksForDir);
  config.writeEntry("worksForFile", _worksForFile);
  config.writeEntry("worksForDotEntry", _worksForDotEntry);
  config.writeEntry("worksLocalOnly", _worksLocalOnly);
  config.writeEntry("recurse", _recurse);
  config.writeEntry("askForConfirmation", _askForConfirmation);
  config.writeEntry("refreshPolicy", (int)_refreshPolicy);
}

