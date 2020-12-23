/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Authors:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 *              Jerome Robert <jeromerobert@gmx.com>
 */

#include <stdlib.h>
#include <time.h>

#include <QDesktopServices>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStyleFactory>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <qcolor.h>
#include <qmenu.h>
#include <qtimer.h>

#include <KLocalizedString>
#include <QApplication>
#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kexcluderules.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <ktoolinvocation.h>

#include "kdirreadjob.h"
#include "kdirtreeview.h"

#define SEPARATE_READ_JOBS_COL 0
#define VERBOSE_PROGRESS_INFO 0

namespace KDirStat {

class KDirModel: public QStandardItemModel {
  KDirTreeView & view_;
  QStringList headers_;

  int numChildren(KFileInfo * f) const {
    return f->numChildren() + (f->dotEntry() ? 1 : 0);
  }

  QStandardItem * createItem(KFileInfo * f) const {
    assert(f != nullptr);
    QStandardItem * newItem = new QStandardItem();
    newItem->setData(QVariant::fromValue(static_cast<void*>(f)));
    newItem->setColumnCount(headers_.size());
    newItem->setEditable(false);
    assert(newItem->data().value<void*>() == f);
    return newItem;
  }

public:
  KDirModel(KDirTreeView * view, QStringList headers):
    QStandardItemModel(view), view_(*view), headers_(headers) {
    setColumnCount(headers_.size());
  }

  bool canFetchMore(const QModelIndex &parent) const override {
    if(parent.isValid()) {
      KFileInfo * f = indexToFile(parent);
      bool finished = f->readState() == KDirFinished;
      return finished && (rowCount(parent) < numChildren(f));
    } else {
      return false;
    }
  }

  bool hasChildren(const QModelIndex &) const override {
    return true;
  }

  void fetchMore(const QModelIndex &parent) override {
    KFileInfo * f = indexToFile(parent);
    int n = numChildren(f);
    QStandardItem * item = itemFromIndex(parent);
    int rc = item->rowCount();
    QList<QStandardItem*> list;
    list.reserve(n-rc);
    for(int i = rc; i < n; i++) {
      KFileInfo * c;
      if(f->dotEntry()) {
        c = i == 0 ? f->dotEntry() : f->child(i - 1);
      } else
        c = f->child(i);
      list.append(createItem(c));
    }
    item->insertRows(rc, list);
    assert(rowCount(parent) == n);
  }

  void removeFile(KFileInfo* file) {
    QModelIndex tr = fileToIndex(file, false);
    removeRow(tr.row(), parent(tr));
  }

  void updateData(QModelIndex r = QModelIndex()) {
    QModelIndex end = sibling(r.row(), columnCount() - 1, r);
    emit dataChanged(r, end);
    for(int i = 0; i < rowCount(r); i++) {
      updateData(index(i, 0, r));
    }
  }

  QModelIndex fileToIndex(KFileInfo* file, bool fetch=true) {
    if(file == view_.tree()->root())
      return indexFromItem(invisibleRootItem()->child(0));
    QModelIndex p = fileToIndex(file->parent(), fetch);
    if(fetch && canFetchMore(p))
      fetchMore(p);
    for(int i = 0; i < rowCount(p); i++) {
      QModelIndex id = index(i, 0, p);
      if(indexToFile(id) == file)
        return id;
    }
    return QModelIndex();
  }

  KFileInfo * indexToFile(const QModelIndex & i) const {
    QStandardItem * si = itemFromIndex(sibling(i.row(), 0, i));
    assert(si != nullptr);
    assert(si != invisibleRootItem());
    void * rv = si->data().value<void*>();
    assert(rv != nullptr);
    return static_cast<KFileInfo*>(rv);
  }

  void setRoot(KDirInfo * root) {
    QStandardItem * r = invisibleRootItem();
    r->removeRows(0, r->rowCount());
    if(root != nullptr)
      r->appendRow(createItem(root));
  }

  QModelIndex getRoot() {
    QModelIndex r = indexFromItem(invisibleRootItem()->child(0));
    if(canFetchMore(r))
      fetchMore(r);
    return r;
  }

  QModelIndexList children(const QModelIndex & mi) {
    QModelIndexList r;
    r.reserve(rowCount(mi));
    for(int i = 0; i < rowCount(mi); i++)
      r.append(index(i, 0, mi));
    return r;
  }

  QVariant dataDisplay(KFileInfo * _orig, int column) const {
    QString prefix = _orig->readState() == KDirAborted ? " >" : " ";
    bool multi = _orig->isDir() || _orig->isDotEntry();
    if(column == view_.percentNumCol()) {
      if (_orig->parent() && // only if there is a parent as calculation base
        _orig->parent()->pendingReadJobs() < 1 && // not before subtree is finished reading
        _orig->parent()->totalSize() > 0 && // avoid division by zero
        !_orig->isExcluded()) // not if this is an excluded object (dir)
      {
        return formatPercent(100. * _orig->totalSize() / _orig->parent()->totalSize());
      } else {
        return "";
      }
    } else if(column == view_.percentBarCol() && _orig->isDir() && _orig->isExcluded()) {
      return i18n("[excluded]");
    } else if(column == view_.totalSubDirsCol() && _orig->isDir()) {
      return " " + formatCount(_orig->totalSubDirs());
    } else if(column == view_.readJobsCol() && multi) {
#if SEPARATE_READ_JOBS_COL
      return " " + formatCount(_orig->pendingReadJobs(), true);
#else
      int jobs = _orig->pendingReadJobs();
      return jobs > 0 ? i18n("[%1 Read Jobs]",
        formatCount(_orig->pendingReadJobs(), true)) : "";
#endif
    } else if(column == view_.totalSizeCol() && multi) {
      return prefix + formatSize(_orig->totalSize());
    } else if(column == view_.totalItemsCol() && multi) {
      return prefix + formatSizeLong(_orig->totalItems());
    } else if(column == view_.totalFilesCol() && multi) {
      return prefix + formatSizeLong(_orig->totalFiles());
    } else if(column == view_.nameCol())
      return _orig->isDotEntry() ? i18n("<Files>") :  _orig->name();
    else if(column == view_.latestMtimeCol()) {
      return localeTimeDate(_orig->latestMtime());
    } else if(column == view_.ownSizeCol() && !_orig->isDevice()) {
      QString text;
      if (_orig->isFile() && _orig->links() > 1) {
        // Regular file with multiple links
        if (_orig->isSparseFile()) {
          text = i18n("%1 / %2 Links (allocated: %3)",
                      formatSize(_orig->byteSize()), formatSize(_orig->links()),
                      formatSize(_orig->allocatedSize()));
        } else {
          text = i18n("%1 / %2 Links", formatSize(_orig->byteSize()),
                      _orig->links());
        }
      } else if(!_orig->isDotEntry()){
        // No multiple links or no regular file
        if (_orig->isSparseFile()) {
          text = i18n("%1 (allocated: %2)", formatSize(_orig->byteSize()),
                      formatSize(_orig->allocatedSize()));
        } else {
          text = formatSize(_orig->size());
        }
      }
      return text;
    }
    return QVariant();
  }

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return headers_[section];
    else
      return QVariant();
  }

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
    KFileInfo * _orig = indexToFile(index);
    if(_orig == nullptr) {
      return QVariant();
    } else if(role == Qt::DisplayRole) {
      return dataDisplay(_orig, index.column());
    } else if(role == Qt::TextAlignmentRole) {
      int c = index.column();
      if(c == view_._totalSizeCol ||
          c == view_._percentNumCol ||
          c == view_._ownSizeCol ||
          c == view_._totalItemsCol ||
          c == view_._totalFilesCol ||
          c == view_._totalSubDirsCol ||
          c == view_._readJobsCol)
          return Qt::AlignRight;
    } else if(role == Qt::DecorationRole && index.column() == 0) {
      QPixmap icon;
      if (_orig->isDotEntry()) {
        icon = view_.isExpanded(index) ? view_.openDotEntryIcon() :
               view_.closedDotEntryIcon();
      } else if (_orig->isDir()) {
        if (_orig->readState() == KDirAborted) {
          icon = view_.stopIcon();
        } else if (_orig->readState() == KDirError) {
          icon = view_.unreadableDirIcon();
        } else {
          if (_orig->isMountPoint()) {
            icon = view_.mountPointIcon();
          } else {
            icon = view_.isExpanded(index) ? view_.openDirIcon() : view_.closedDirIcon();
          }
        }
      } else if (_orig->isFile())
        icon = view_.fileIcon();
      else if (_orig->isSymLink())
        icon = view_.symLinkIcon();
      else if (_orig->isBlockDevice())
        icon = view_.blockDevIcon();
      else if (_orig->isCharDevice())
        icon = view_.charDevIcon();
      else if (_orig->isSpecial())
        icon = view_.fifoIcon();
      return icon;
    }
    return QVariant();
  }
};

class KDirSortFilterProxyModel : public QSortFilterProxyModel {
// https://doc.qt.io/qt-5/qtwidgets-itemviews-customsortfiltermodel-example.html
public:
  KDirSortFilterProxyModel(KDirTreeView * view): QSortFilterProxyModel(view) {}
protected:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
    KDirTreeView * _view = static_cast<KDirTreeView *>(parent());
    KDirModel * m = static_cast<KDirModel*>(sourceModel());
    KFileInfo * _orig = m->indexToFile(left);
    KFileInfo * otherOrig = m->indexToFile(right);
    assert(left.column() == right.column());
    int column = left.column();
    if (column == _view->totalSizeCol() || column == _view->percentNumCol() ||
      column == _view->percentBarCol())
      return _orig->totalSize() > otherOrig->totalSize();
    else if (column == _view->ownSizeCol())
      return _orig->size() > otherOrig->size();
    else if (column == _view->totalItemsCol())
      return _orig->totalItems() > otherOrig->totalItems();
    else if (column == _view->totalFilesCol())
      return _orig->totalFiles() > otherOrig->totalFiles();
    else if (column == _view->totalSubDirsCol())
      return _orig->totalSubDirs() > otherOrig->totalSubDirs();
    else if (column == _view->latestMtimeCol())
      return _orig->latestMtime() > otherOrig->latestMtime();
    else if (_orig->isDotEntry()) // make sure dot entries are last in the list
      return true;
    else if (otherOrig->isDotEntry())
      return false;
    assert(false);
    return false;
  }
};

/** @brief Rendering of the percentage bar */
class KDirItemDelegate : public QStyledItemDelegate {
public:
  KDirItemDelegate(KDirTreeView *view) : QStyledItemDelegate(view), view(view) {
    // always use the Fusion style because it has the best
    // progress bar precision
    style = QStyleFactory::create("Fusion");
    style->setParent(this);
  }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override {
    QModelIndex srcIdx = view->proxyModel()->mapToSource(index);
    KFileInfo * item = view->model()->indexToFile(srcIdx);
    if (view->readJobsCol() == view->percentBarCol()) {
      if (item->pendingReadJobs() > 0) {
        QString t =
            i18n("[%1 Read Jobs]", formatCount(item->pendingReadJobs(), true));
        QApplication::style()->drawItemText(
            painter, option.rect, Qt::AlignRight, view->palette(), true, t);
      }
    } else if (item->treeLevel() > 0) {
      QStyleOptionProgressBar o;
      o.rect = option.rect;
      o.minimum = 0;
      o.maximum = 100;
      o.progress = 100 * item->totalSize() / item->parent()->totalSize();
      o.palette.setColor(QPalette::Highlight,
                         view->fillColor(item->treeLevel() - 1));
      if (view->selection() != item)
        o.palette.setColor(QPalette::Base, view->palette().base().color());
      style->drawControl(QStyle::CE_ProgressBar, &o, painter);
    }
  }

private:
  QStyle *style;
  KDirTreeView *view;
};

KFileInfo * KDirTreeView::selection() const {
  QModelIndexList indices = selectedIndexes();
  if(indices.empty())
    return nullptr;
  else {
    QModelIndex idx = proxyModel()->mapToSource(indices[0]);
    return model()->indexToFile(idx);
  }
}

QSortFilterProxyModel * KDirTreeView::proxyModel() const {
  return static_cast<QSortFilterProxyModel*>(QTreeView::model());
}

KDirModel * KDirTreeView::model() const {
  return static_cast<KDirModel*>(proxyModel()->sourceModel());
}

KDirTreeView::KDirTreeView(QWidget *parent):
  QTreeView(parent), _tree(new KDirTree()) {
  _updateTimer = 0;
  _openLevel = 1;
  _doLazyClone = true;
  _doPacManAnimation = false;
  _updateInterval = 333; // millisec

  for (int i = 0; i < DEBUG_COUNTERS; i++)
    _debugCount[i] = 0;

  setDebugFunc(1, "KDirTreeViewItem::init()");
  setDebugFunc(2, "KDirTreeViewItem::updateSummary()");
  setDebugFunc(3, "KDirTreeViewItem::deferredClone()");
  setDebugFunc(4, "KDirTreeViewItem::compare()");
  setDebugFunc(5, "KDirTreeViewItem::paintCell()");

#if SEPARATE_READ_JOBS_COL
  _readJobsCol = -1;
#endif
  setRootIsDecorated(false);

  int numCol = 0;
  QStringList colLabels;
  colLabels << i18n("Name");
  _nameCol = numCol;
  _iconCol = numCol++;
  colLabels << i18n("Subtree Percentage");
  _percentBarCol = numCol++;
  colLabels << i18n("Percentage");
  _percentNumCol = numCol++;
  colLabels << i18n("Subtree Total");
  _totalSizeCol = numCol++;
  _workingStatusCol = _totalSizeCol;
  colLabels << i18n("Own Size");
  _ownSizeCol = numCol++;
  colLabels << i18n("Items");
  _totalItemsCol = numCol++;
  colLabels << i18n("Files");
  _totalFilesCol = numCol++;
  colLabels << i18n("Subdirs");
  _totalSubDirsCol = numCol++;
  colLabels << i18n("Last Change");
  _latestMtimeCol = numCol++;

#if !SEPARATE_READ_JOBS_COL
  _readJobsCol = _percentBarCol;
#endif
  sortByColumn(_totalSizeCol, Qt::AscendingOrder);
  setItemDelegateForColumn(_percentBarCol, new KDirItemDelegate(this));
#define loadIcon(ICON)                                                         \
  KIconLoader::global()->loadIcon((ICON), KIconLoader::Small)

  _openDirIcon = loadIcon("folder-open");
  _closedDirIcon = loadIcon("folder");
  _openDotEntryIcon = loadIcon("folder-orange");
  _closedDotEntryIcon = loadIcon("folder-orange");
  _unreadableDirIcon = loadIcon("folder-locked");
  _mountPointIcon = loadIcon("drive-harddisk");
  _fileIcon = loadIcon("mime_empty");
  _symLinkIcon =
      loadIcon("emblem-symbolic-link"); // The KDE standard link icon is ugly!
  _blockDevIcon = loadIcon("blockdevice");
  _charDevIcon = loadIcon("chardevice");
  _fifoIcon = loadIcon("socket");
  _stopIcon = loadIcon("process-stop");
  _readyIcon = QPixmap();

#undef loadIcon
  setSortingEnabled(true);
  setUniformRowHeights(true);

  setDefaultFillColors();
  readConfig();
  ensureContrast();

  // Does not work for GTK style
  QGuiApplication *app =
      dynamic_cast<QGuiApplication *>(QCoreApplication::instance());
  connect(app, SIGNAL(paletteChanged(const QPalette &)), this,
          SLOT(paletteChanged()));

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(popupContextMenu(const QPoint &)));

  connect(this, SIGNAL(expanded(const QModelIndex &)), this,
          SLOT(resizeIndexToContents(const QModelIndex &)));
  connect(this, SIGNAL(collapsed(const QModelIndex &)), this,
          SLOT(resizeIndexToContents(const QModelIndex &)));

  _contextInfo = new QMenu(this);
  infoAction = new QAction(_contextInfo);
  _contextInfo->addAction(infoAction);
  QSortFilterProxyModel *proxyModel = new KDirSortFilterProxyModel(this);
  proxyModel->setSourceModel(new KDirModel(this, colLabels));
  setModel(proxyModel);
  createTree();
  setSelectionMode(QAbstractItemView::ExtendedSelection);
}

KDirTreeView::~KDirTreeView() {
  delete _tree;

  /*
   * Don't delete _updateTimer here, it's already automatically deleted by Qt!
   * (Since it's derived from QObject and has a QObject parent).
   */
}

void KDirTreeView::setDebugFunc(int i, const QString &functionName) {
  if (i > 0 && i < DEBUG_COUNTERS)
    _debugFunc[i] = functionName;
}

void KDirTreeView::incDebugCount(int i) {
  if (i > 0 && i < DEBUG_COUNTERS)
    _debugCount[i]++;
}

void KDirTreeView::busyDisplay() {
#if SEPARATE_READ_JOBS_COL
  if (_readJobsCol < 0) {
    _readJobsCol = header()->count();
    addColumn(i18n("Read Jobs"));
    setColumnAlignment(_readJobsCol, AlignRight);
  }
#else
  _readJobsCol = _percentBarCol;
#endif
}

void KDirTreeView::idleDisplay() {
#if SEPARATE_READ_JOBS_COL
  if (_readJobsCol >= 0) {
    removeColumn(_readJobsCol);
  }
#else
  if (proxyModel()->sortColumn() == _readJobsCol && proxyModel()->sortColumn() >= 0) {
    // A pathological case: The user requested sorting by read jobs, and
    // now that everything is read, the items are still in that sort order.
    // Not only is that sort order now useless (since all read jobs are
    // done), it is contrary to the (now changed) semantics of this
    // column. Calling QListView::sort() might do the trick, but we can
    // never know just how clever that QListView widget tries to be and
    // maybe avoid another sorting by the same column - so let's use the
    // easy way out and sort by another column that has the same sorting
    // semantics like the percentage bar column (that had doubled as the
    // read job column while reading) now has.

    sortByColumn(_percentNumCol);
  }
#endif
  _readJobsCol = -1;
}

void KDirTreeView::openURL(QUrl url) {
  clear();
  _tree->clear();

  // Implicitly calling prepareReading() via the tree's startingReading() signal
  _tree->startReading(url);

  logActivity(30);
}

void KDirTreeView::createTree() {
  // Clean up any old leftovers

  clear();
  _currentDir = "";

  // Connect signals

  connect(_tree, SIGNAL(progressInfo(const QString &)), this,
          SLOT(sendProgressInfo(const QString &)));

  connect(_tree, SIGNAL(childAdded(KFileInfo *)), this,
          SLOT(slotAddChild(KFileInfo *)));

  connect(_tree, SIGNAL(deletingChild(KFileInfo *)), this,
          SLOT(deleteChild(KFileInfo *)));

  connect(_tree, SIGNAL(startingReading()), this, SLOT(prepareReading()));

  connect(_tree, SIGNAL(finished()), this, SLOT(slotFinished()));

  connect(_tree, SIGNAL(aborted()), this, SLOT(slotAborted()));

  connect(_tree, SIGNAL(finalizeLocal(KDirInfo *)), this,
          SLOT(finalizeLocal(KDirInfo *)));

  connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          this, SLOT(fileSelectionChanged(const QItemSelection&, const QItemSelection&)));

  connect(_tree, SIGNAL(selectionChanged(KDirTree*)), this,
          SLOT(updateSelection(KDirTree *)));
}

void KDirTreeView::prepareReading() {
  // Prepare cyclic update

  if (_updateTimer)
    delete _updateTimer;

  _updateTimer = new QTimer(this);

  if (_updateTimer) {
    _updateTimer->setInterval(_updateInterval);
    connect(_updateTimer, SIGNAL(timeout()), this, SLOT(updateSummary()));

    connect(_updateTimer, SIGNAL(timeout()), this, SLOT(sendProgressInfo()));
    _updateTimer->start();
  }

  // Change display to busy state

  sortByColumn(_totalSizeCol);
  busyDisplay();
  emit startingReading();

  _stopWatch.start();
}

void KDirTreeView::refreshAll() {
  if (_tree->root()) {
    clear();
    // Implicitly calling prepareReading() via the tree's startingReading()
    // signal
    _tree->refresh(0);
  }
}

void KDirTreeView::abortReading() {
  _tree->abortReading();
}

void KDirTreeView::clear() {
  selectionModel()->clearSelection();
  model()->setRoot(nullptr);
  for (int i = 0; i < DEBUG_COUNTERS; i++)
    _debugCount[i] = 0;
}

bool KDirTreeView::writeCache(const QString &cacheFileName) {
  return _tree->writeCache(cacheFileName);
}

void KDirTreeView::readCache(const QString &cacheFileName) {
  clear();
  _tree->clear();
  _tree->readCache(cacheFileName);
}

void KDirTreeView::slotAddChild(KFileInfo * f) {
  if(f != _tree->root()) {
    QModelIndex idx = model()->fileToIndex(f->parent(), false);
    QModelIndex proxyIdx = proxyModel()->mapFromSource(idx);
    if(idx.isValid() && isExpanded(proxyIdx) && model()->canFetchMore(idx)) {
      model()->fetchMore(idx);
    }
  }
}

void KDirTreeView::deleteChild(KFileInfo *clone) {
 /**
  * The selected item is about to be deleted. Select some other item
  * so there is still something selected: Preferably the next item
  * or the parent if there is no next. This cannot be done from
  * outside because the order of items is not known to the outside;
  * it might appear very random if the next item in the KFileInfo
  * list would be selected. The order of that list is definitely
  * different than the order of this view - which is what the user
  * sees. So let's give the user a reasonable next selection so he
  * can continue working without having to explicitly select another
  * item.
  *
  * This is very useful if the user just activated a cleanup action
  * that deleted an item: It makes sense to implicitly select the
  * next item so he can clean up many items in a row.
  **/
  QModelIndex nextSelection;
  QModelIndexList indices = selectedIndexes();
  std::sort(indices.begin(), indices.end());
  for(int i = 0; i < indices.length(); i++) {
    QModelIndex mi = proxyModel()->mapToSource(indices[i]);
    if(model()->indexToFile(mi) == clone)
      nextSelection = proxyModel()->sibling(indices[i].row() + 1, indices[i].column(), indices[i]);
  }
  if(nextSelection.isValid()) {
    setCurrentIndex(nextSelection);
  }
  model()->removeFile(clone);
}

void KDirTreeView::updateSummary() {
  model()->updateData();
  bool se = isSortingEnabled();
  setSortingEnabled(false);
  for (int column = 0; column < this->model()->columnCount(); column++) {
    resizeColumnToContents(column);
  }
  setSortingEnabled(se);
}

void KDirTreeView::slotFinished() {
  emit progressInfo(i18n("Finished. Elapsed time: %1",
                         formatTime(_stopWatch.elapsed(), true)));

  if (_updateTimer) {
    delete _updateTimer;
    _updateTimer = 0;
  }

  updateSummary();
  idleDisplay();
  logActivity(30);

  if (_tree->root() && _tree->root()->totalSubDirs() == 0 && // No subdirs
      _tree->root()->totalItems() > 0) // but file children
  {
    setExpanded(QModelIndex(), true);
  }

#if 0
    for ( int i=0; i < DEBUG_COUNTERS; i++ )
    {
	qDebug() << "Debug counter #" << i << ": " << _debugCount[i]
		  << "\t" << _debugFunc[i]
		  << endl;
    }
    qDebug() << endl;
#endif

  emit finished();
}

void KDirTreeView::slotAborted() {
  emit progressInfo(i18n("Aborted. Elapsed time: %1",
                         formatTime(_stopWatch.elapsed(), true)));

  if (_updateTimer) {
    delete _updateTimer;
    _updateTimer = 0;
  }

  idleDisplay();
  updateSummary();

  emit aborted();
}

void KDirTreeView::finalizeLocal(KDirInfo *dir) {
  if(dir == _tree->root()) {
    model()->setRoot(dir);
    setExpanded(proxyModel()->mapFromSource(model()->getRoot()), true);
  }
}

void KDirTreeView::sendProgressInfo(const QString &newCurrentDir) {
  _currentDir = newCurrentDir;

#if VERBOSE_PROGRESS_INFO
  emit progressInfo(i18n("Elapsed time: %1   reading directory %2",
                         formatTime(_stopWatch.elapsed()), _currentDir));
#else
  emit progressInfo(i18n("Elapsed time: %1", formatTime(_stopWatch.elapsed())));
#endif
}

void KDirTreeView::fileSelectionChanged(const QItemSelection &, const QItemSelection &) {
  if(this->bypassTreeSelection) {
    return;
  }
  std::vector<KFileInfo *> newSelection;
  QList<QModelIndex> idxs = selectedIndexes();
  for(int i = 0; i < idxs.size(); i++) {
    QModelIndex idx = proxyModel()->mapToSource(idxs[i]);
    if(idx.column() == 0)
      newSelection.push_back(model()->indexToFile(idx));
  }
  this->bypassTreeSelection = true;
  tree()->selectItems(newSelection);
  this->bypassTreeSelection = false;
}

void KDirTreeView::updateSelection(KDirTree * tree) {
  if(this->bypassTreeSelection)
    return;
  // Short-circuit for the most common case: The signal has been triggered by
  // this view, and the KDirTree has sent it right back.
  QSet<QModelIndex> newSelSet, curSelSet;
  for(size_t i = 0; i < tree->selection().size(); i++) {
    QModelIndex pIdx = proxyModel()->mapFromSource(
          model()->fileToIndex(tree->selection()[i]));
    newSelSet.insert(pIdx);
  }
  const QItemSelection & curSel = selectionModel()->selection();
  for(int i = 0; i < curSel.size(); i++) {
    QList<QModelIndex> idxs = curSel[i].indexes();
    for(int j = 0; j < idxs.size(); j++) {
      if(idxs[j].column() == 0)
        curSelSet.insert(idxs[j]);
    }
  }

  if (newSelSet != curSelSet) {
    QItemSelection newSel;
    auto it = newSelSet.begin();
    if(it != newSelSet.end()) {
      assert(it->isValid());
      assert(it->model() == proxyModel());
      if(newSelSet.size() == 1) {
        collapseAll();
        selectionModel()->setCurrentIndex(*it, QItemSelectionModel::NoUpdate);
      }
      while(it != newSelSet.end()) {
        setExpanded(*it, true);
        newSel.select(*it, *it);
        ++it;
      }
    }
    this->bypassTreeSelection = true;
    selectionModel()->select(newSel, QItemSelectionModel::ClearAndSelect |
                             QItemSelectionModel::Current | QItemSelectionModel::Rows);
    this->bypassTreeSelection = false;
  }
}

const QColor &KDirTreeView::fillColor(int level) const {
  if (level < 0) {
    qWarning() << Q_FUNC_INFO << "Invalid argument: " << level << Qt::endl;
    level = 0;
  }

  return _fillColor[level % _usedFillColors];
}

const QColor &KDirTreeView::rawFillColor(int level) const {
  if (level < 0 || level > KDirTreeViewMaxFillColor) {
    level = 0;
    qWarning() << Q_FUNC_INFO << "Invalid argument: " << level << Qt::endl;
  }

  return _fillColor[level % KDirTreeViewMaxFillColor];
}

void KDirTreeView::setFillColor(int level, const QColor &color) {
  if (level >= 0 && level < KDirTreeViewMaxFillColor)
    _fillColor[level] = color;
}

void KDirTreeView::setUsedFillColors(int usedFillColors) {
  if (usedFillColors < 1) {
    qWarning() << Q_FUNC_INFO << "Invalid argument: " << usedFillColors << Qt::endl;
    usedFillColors = 1;
  } else if (usedFillColors >= KDirTreeViewMaxFillColor) {
    qWarning() << Q_FUNC_INFO << "Invalid argument: " << usedFillColors
               << " (max: " << KDirTreeViewMaxFillColor - 1 << ")" << Qt::endl;
    usedFillColors = KDirTreeViewMaxFillColor - 1;
  }

  _usedFillColors = usedFillColors;
}

void KDirTreeView::setDefaultFillColors() {
  int i;

  for (i = 0; i < KDirTreeViewMaxFillColor; i++) {
    _fillColor[i] = Qt::blue;
  }

  i = 0;
  _usedFillColors = 4;

  setFillColor(i++, QColor(0, 0, 255));
  setFillColor(i++, QColor(128, 0, 128));
  setFillColor(i++, QColor(231, 147, 43));
  setFillColor(i++, QColor(4, 113, 0));
  setFillColor(i++, QColor(176, 0, 0));
  setFillColor(i++, QColor(204, 187, 0));
  setFillColor(i++, QColor(162, 98, 30));
  setFillColor(i++, QColor(0, 148, 146));
  setFillColor(i++, QColor(217, 94, 0));
  setFillColor(i++, QColor(0, 194, 65));
  setFillColor(i++, QColor(194, 108, 187));
  setFillColor(i++, QColor(0, 179, 255));
}

void KDirTreeView::setTreeBackground(const QColor &color) {
  _treeBackground = color;
  _percentageBarBackground = _treeBackground.dark(115);

  QGuiApplication *app =
      dynamic_cast<QGuiApplication *>(QCoreApplication::instance());
  QPalette pal = app->palette();
  pal.setBrush(QPalette::Base, _treeBackground);
  setPalette(pal);
}

void KDirTreeView::ensureContrast() {
  if (palette().base() == Qt::white || palette().base() == Qt::black)
    setTreeBackground(palette().midlight().color());
  else
    setTreeBackground(palette().base().color());
}

void KDirTreeView::paletteChanged() {
  setTreeBackground(KColorScheme::ActiveBackground);
  ensureContrast();
}

void KDirTreeView::popupContextMenu(const QPoint &localPos) {
  QModelIndex idx = indexAt(localPos);
  int column = columnAt(localPos.x());
  QPoint pos = viewport()->mapToGlobal(localPos);
  KFileInfo *orig = model()->indexToFile(proxyModel()->mapToSource(idx));

  if (!orig) {
    qCritical() << "NULL item->orig()" << Qt::endl;
    return;
  }

  if (column == _nameCol || column == _percentBarCol ||
      column == _percentNumCol) {
    if (orig->isExcluded() && column == _percentBarCol) {
      // Show with exclude rule caused the exclusion

      const KExcludeRule *rule =
          KExcludeRules::excludeRules()->matchingRule(orig->url());

      QString text;

      if (rule) {
        text = i18n("Matching exclude rule:   %1", rule->regexp().pattern());
      } else {
        text = i18n("<Unknown exclude rule>");
      }

      popupContextInfo(pos, text);
    } else {
      // Make the item the context menu is popping up over the current
      // selection - all user operations refer to the current selection.
      // Just right-clicking on an item does not make it the current
      // item!
      if(selectedIndexes().size() < 2)
        setCurrentIndex(idx);

      // Let somebody from outside pop up the context menu, if so desired.
      emit contextMenu(pos);
    }
  }

  // If the column is one with a large size in kB/MB/GB, open a
  // info popup with the exact number.

  if (column == _ownSizeCol && !orig->isDotEntry()) {
    if (orig->isSparseFile() || (orig->links() > 1 && orig->isFile())) {
      QString text;

      if (orig->isSparseFile()) {
        text =
            i18n("Sparse file: %1 (%2 Bytes) -- allocated: %3 (%4 Bytes)",
                 formatSize(orig->byteSize()), formatSizeLong(orig->byteSize()),
                 formatSize(orig->allocatedSize()),
                 formatSizeLong(orig->allocatedSize()));
      } else {
        text = i18n(
            "%1 (%2 Bytes) with %3 hard links => effective size: %4 (%5 Bytes)",
            formatSize(orig->byteSize()), formatSizeLong(orig->byteSize()),
            orig->links(), formatSize(orig->size()),
            formatSizeLong(orig->size()));
      }

      popupContextInfo(pos, text);
    } else {
      popupContextSizeInfo(pos, orig->size());
    }
  }

  if (column == _totalSizeCol &&
      (orig->isDir() || orig->isDotEntry())) {
    popupContextSizeInfo(pos, orig->totalSize());
  }

  // Show alternate time / date format in time / date related columns.

  if (column == _latestMtimeCol) {
    popupContextInfo(pos, formatTimeDate(orig->latestMtime()));
  }

  logActivity(3);
}

void KDirTreeView::popupContextSizeInfo(const QPoint &pos, KFileSize size) {
  QString info;

  if (size < 1024) {
    info = formatSizeLong(size) + " " + i18n("Bytes");
  } else {
    info = i18n("%1 (%2 Bytes)", formatSize(size), formatSizeLong(size));
  }

  popupContextInfo(pos, info);
}

void KDirTreeView::popupContextInfo(const QPoint &pos, const QString &info) {
  infoAction->setText(info);
  _contextInfo->popup(pos);
}

void KDirTreeView::readConfig() {
  KConfigGroup config = KSharedConfig::openConfig()->group("Tree Colors");
  _usedFillColors = config.readEntry("usedFillColors", -1);

  if (_usedFillColors < 0) {
    /*
     * No 'usedFillColors' in the config file?  Better forget that
     * file and use default values. Otherwise, all colors would very
     * likely become blue - the default color.
     */
    setDefaultFillColors();
  } else {
    // Read the rest of the 'Tree Colors' section

    QColor defaultColor(Qt::blue);

    for (int i = 0; i < KDirTreeViewMaxFillColor; i++) {
      QString name;
      name.sprintf("fillColor_%02d", i);
      _fillColor[i] = config.readEntry(name, defaultColor);
    }
  }

  if (isVisible())
    viewport()->repaint();
}

void KDirTreeView::saveConfig() const {
  KConfigGroup config = KSharedConfig::openConfig()->group("Tree Colors");

  config.writeEntry("usedFillColors", _usedFillColors);

  for (int i = 0; i < KDirTreeViewMaxFillColor; i++) {
    QString name;
    name.sprintf("fillColor_%02d", i);
    config.writeEntry(name, _fillColor[i]);
  }
}

void KDirTreeView::logActivity(int points) { emit userActivity(points); }

QString KDirTreeView::asciiDump(QModelIndex & idx) const {
  KFileInfo * _orig = model()->indexToFile(proxyModel()->mapToSource(idx));
  QString dump = QString("%1 %2")
      .arg(formatSize( _orig->totalSize()))
      .arg(_orig->debugUrl());

  if ( isExpanded(idx) ) {
    for(int i = 0; i < proxyModel()->rowCount(idx); i++) {
      QModelIndex sidx = proxyModel()->index(i, 0, idx);
      dump += asciiDump(sidx);
    }
  }

  return dump;
}

void KDirTreeView::sendMailToOwner() {
  QModelIndexList indices = selectedIndexes();
  if (indices.empty()) {
    qCritical() << Q_FUNC_INFO << "Nothing selected!" << Qt::endl;
    return;
  }
  QModelIndex idx = indices.at(0);
  KFileInfo * orig = model()->indexToFile(proxyModel()->mapToSource(idx));
  QString owner = KioDirReadJob::owner(fixedUrl(orig->url()));
  QString subject = i18n("Disk Usage");
  QString body =
      i18n("Please check your disk usage and clean up if you can. Thank you.") +
      "\n\n" + asciiDump(idx) + "\n\n" +
      i18n("Disk usage report generated by k4dirstat") +
      "\nhttps://github.com/jeromerobert/k4dirstat/";

  // qDebug() << "owner: "   << owner   << endl;
  // qDebug() << "subject: " << subject << endl;
  // qDebug() << "body:\n"   << body    << endl;

  QUrl mail;
  mail.setScheme("mailto");
  mail.setPath(owner);
  mail.setQuery("?subject=" + QUrl::toPercentEncoding(subject) +
                "&body=" + QUrl::toPercentEncoding(body));

  // TODO: Check for maximum command line length.
  //
  // The hard part with this is how to get this from all that 'autoconf'
  // stuff into 'config.h' or some other include file without hardcoding
  // anything - this is too system dependent.

  QDesktopServices::openUrl(mail);
  logActivity(10);
}

void KDirTreeView::resizeIndexToContents(const QModelIndex &index) {
  resizeColumnToContents(index.column());
}


QString formatSizeLong(KFileSize size) {
  return QLocale().toString(size);
}

QString hexKey(KFileSize size) {
  /**
   * This is optimized for performance, not for aesthetics.
   * And every now and then the old C hacker breaks through in most of us...
   * ;-)
   **/

  static const char hexDigits[] = "0123456789ABCDEF";
  char key[sizeof(KFileSize) * 2 + 1]; // 2 hex digits per byte required
  char *cptr = key + sizeof(key) - 1;  // now points to last char of key

  memset(key, '0', sizeof(key) - 1); // fill with zeroes
  *cptr-- = 0;                       // terminate string

  while (size > 0) {
    *cptr-- = hexDigits[size & 0xF]; // same as size % 16
    size >>= 4;                      // same as size /= 16
  }

  return QString(key);
}

QString formatTime(long millisec, bool showMilliSeconds) {
  QString formattedTime;
  int hours;
  int min;
  int sec;

  hours = millisec / 3600000L; // 60*60*1000
  millisec %= 3600000L;

  min = millisec / 60000L; // 60*1000
  millisec %= 60000L;

  sec = millisec / 1000L;
  millisec %= 1000L;

  if (showMilliSeconds) {
    formattedTime.sprintf("%02d:%02d:%02d.%03ld", hours, min, sec, millisec);
  } else {
    formattedTime.sprintf("%02d:%02d:%02d", hours, min, sec);
  }

  return formattedTime;
}

QString formatCount(int count, bool suppressZero) {
  if (suppressZero && count == 0)
    return "";

  QString countString;
  countString.setNum(count);

  return countString;
}

QString formatPercent(float percent) {
  QString percentString;

  percentString.sprintf("%.1f%%", percent);

  return percentString;
}

QString formatTimeDate(time_t rawTime) {
  QString timeDateString;
  struct tm *t = localtime(&rawTime);

  /*
   * Format this as "yyyy-mm-dd hh:mm:ss".
   *
   * This format may not be POSIX'ly correct, but it is the ONLY of all those
   * brain-dead formats today's computer users are confronted with that makes
   * any sense to the average human.
   *
   * Agreed, it takes some getting used to, too, but once you got that far,
   * you won't want to miss it.
   *
   * Who the hell came up with those weird formats like described in the
   * ctime() man page? Don't those people ever actually use that?
   *
   * What sense makes a format like "Wed Jun 30 21:49:08 1993" ?
   * The weekday (of all things!) first, then a partial month name, then the
   * day of month, then the time and then - at the very end - the year.
   * IMHO this is maximum brain-dead. Not only can't you do any kind of
   * decent sorting or automatic processing with that disinformation
   * hodge-podge, your brain runs in circles trying to make sense of it.
   *
   * I could put up with crap like that if the Americans and Brits like it
   * that way, but unfortunately I as a German am confronted with that
   * bullshit, too, on a daily basis - either because some localization stuff
   * didn't work out right (again) or because some jerk decided to emulate
   * this stuff in the German translation, too. I am sick and tired with
   * that, and since this is MY program I am going to use a format that makes
   * sense to ME.
   *
   * No, no exceptions for Americans or Brits. I had to put up with their
   * crap long enough, now it's time for them to put up with mine.
   * Payback time - though luck, folks.
   * ;-)
   *
   * Stefan Hundhammer <sh@suse.de>	2001-05-28
   * (in quite some fit of frustration)
   */
  timeDateString.sprintf("%4d-%02d-%02d  %02d:%02d:%02d", t->tm_year + 1900,
                         t->tm_mon +
                             1, // another brain-dead common pitfall - 0..11
                         t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

  return timeDateString;
}

QString localeTimeDate(time_t rawTime) {
  QDateTime timeDate;
  timeDate.setTime_t(rawTime);
  return timeDate.toString(Qt::DefaultLocaleShortDate);
}

QColor contrastingColor(const QColor &desiredColor,
                                  const QColor &contrastColor) {
  if (desiredColor != contrastColor) {
    return desiredColor;
  }

  if (contrastColor != contrastColor.light()) {
    // try a little lighter
    return contrastColor.light();
  } else {
    // try a little darker
    return contrastColor.dark();
  }
}

} //namespace
