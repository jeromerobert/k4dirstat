/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include <qevent.h>
#include <qregexp.h>

#include <KSharedConfig>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "kdirtree.h"
#include "ktreemaptile.h"
#include "ktreemapview.h"

using namespace KDirStat;

KTreemapView::KTreemapView(KDirTree *tree, QWidget *parent,
                           const QSize &initialSize)
    : QGraphicsView(parent), _tree(tree), _rootTile(0), _selectedTile(0),
      _selectionRect(0) {
  // qDebug() << Q_FUNC_INFO << endl;

  readConfig();

  // Default values for light sources taken from Wiik / Wetering's paper
  // about "cushion treemaps".

  _lightX = 0.09759;
  _lightY = 0.19518;
  _lightZ = 0.9759;

  if (_autoResize) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }

  if (initialSize.isValid())
    resize(initialSize);

  if (tree && tree->root()) {
    if (!_rootTile) {
      // The treemap might already be created indirectly by
      // rebuildTreemap() called from resizeEvent() triggered by resize()
      // above. If this is so, don't do it again.

      rebuildTreemap(tree->root());
    }
  }
  _refreshTimer.setSingleShot(true);

  connect(tree, SIGNAL(selectionChanged(KDirTree*)), this,
          SLOT(updateSelection(KDirTree *)));

  connect(tree, SIGNAL(deletingChild(KFileInfo *)), this,
          SLOT(deleteNotify(KFileInfo *)));

  connect(tree, SIGNAL(childDeleted()), &_refreshTimer, SLOT(start()));
  connect(&_refreshTimer, SIGNAL(timeout()), this, SLOT(rebuildTreemap()));
}

KTreemapView::~KTreemapView() {}

void KTreemapView::clear() {
  if (scene())
    scene()->clear();
  _selectedTile = 0;
  _selectionRect = 0;
  _rootTile = 0;
}

void KTreemapView::readConfig() {
  KConfigGroup config = KSharedConfig::openConfig()->group("Treemaps");

  _ambientLight = config.readEntry("AmbientLight", DefaultAmbientLight);

  _heightScaleFactor =
      config.readEntry("HeightScaleFactor", DefaultHeightScaleFactor);
  _autoResize = config.readEntry("AutoResize", true);
  _squarify = config.readEntry("Squarify", true);
  _doCushionShading = config.readEntry("CushionShading", true);
  _ensureContrast = config.readEntry("EnsureContrast", true);
  _forceCushionGrid = config.readEntry("ForceCushionGrid", false);
  _minTileSize = config.readEntry("MinTileSize", DefaultMinTileSize);

  _highlightColor = readColorEntry(&config, "HighlightColor", QColor(Qt::red));
  _cushionGridColor =
      readColorEntry(&config, "CushionGridColor", QColor(0x80, 0x80, 0x80));
  _outlineColor = readColorEntry(&config, "OutlineColor", QColor(Qt::black));
  _fileFillColor =
      readColorEntry(&config, "FileFillColor", QColor(0xde, 0x8d, 0x53));
  _dirFillColor =
      readColorEntry(&config, "DirFillColor", QColor(0x10, 0x7d, 0xb4));

  if (_autoResize) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  } else {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }
}

QColor KTreemapView::readColorEntry(KConfigGroup *config, const char *entryName,
                                    QColor defaultColor) {
  return config->readEntry(entryName, defaultColor);
}

KTreemapTile *KTreemapView::tileAt(QPoint pos) {
  KTreemapTile *tile = 0;
  foreach (QGraphicsItem *it, items(pos)) {
    tile = dynamic_cast<KTreemapTile *>(it);
    if (tile)
      break;
  }

  return tile;
}

void KTreemapView::mousePressEvent(QMouseEvent *event) {
  // qDebug() << Q_FUNC_INFO << endl;

  KTreemapTile *tile = tileAt(event->pos());

  if (!tile)
    return;

  switch (event->button()) {
  case Qt::LeftButton:
    selectTile(tile);
    emit userActivity(1);
    break;

  case Qt::MidButton:
    // Select clicked tile's parent, if available

    if (_selectedTile && _selectedTile->rect().contains(event->pos())) {
      if (_selectedTile->parentTile())
        tile = _selectedTile->parentTile();
    }

    // Intentionally handling the middle button like the left button if
    // the user clicked outside the (old) selected tile: Simply select
    // the clicked tile. This makes using this middle mouse button
    // intuitive: It can be used very much like the left mouse button,
    // but it has added functionality. Plus, it cycles back to the
    // clicked tile if the user has already clicked all the way up the
    // hierarchy (i.e. the topmost directory is highlighted).

    selectTile(tile);
    emit userActivity(1);
    break;

  case Qt::RightButton:

    if (tile) {
      if (_selectedTile && _selectedTile->rect().contains(event->pos())) {
        // If a directory (non-leaf tile) is already selected,
        // don't override this by

        emit contextMenu(_selectedTile, event->globalPos());
      } else {
        selectTile(tile);
        emit contextMenu(tile, event->globalPos());
      }

      emit userActivity(3);
    }
    break;

  default:
    // event->button() is an enum, so g++ complains
    // if there are unhandled cases.
    break;
  }
}

void KTreemapView::contentsMouseDoubleClickEvent(QMouseEvent *event) {
  // qDebug() << Q_FUNC_INFO << endl;

  KTreemapTile *tile = tileAt(event->pos());

  if (!tile)
    return;

  switch (event->button()) {
  case Qt::LeftButton:
    if (tile) {
      selectTile(tile);
      zoomIn();
      emit userActivity(5);
    }
    break;

  case Qt::MidButton:
    zoomOut();
    emit userActivity(5);
    break;

  case Qt::RightButton:
    // Double-clicking the right mouse button is pretty useless - the
    // first click opens the context menu: Single clicks are always
    // delivered first. Even if that would be caught by using timers,
    // it would still be very awkward to use: Click too slow, and
    // you'll get the context menu rather than what you really wanted -
    // then you'd have to get rid of the context menu first.
    break;

  default:
    // Prevent compiler complaints about missing enum values in switch
    break;
  }
}

void KTreemapView::zoomIn() {
  if (!_selectedTile || !_rootTile)
    return;

  KTreemapTile *newRootTile = _selectedTile;

  while (newRootTile->parentTile() != _rootTile &&
         newRootTile->parentTile()) // This should never happen, but who knows?
  {
    newRootTile = newRootTile->parentTile();
  }

  if (newRootTile) {
    KFileInfo *newRoot = newRootTile->orig();

    if (newRoot->isDir() || newRoot->isDotEntry())
      rebuildTreemap(newRoot);
  }
}

void KTreemapView::zoomOut() {
  if (_rootTile) {
    KFileInfo *root = _rootTile->orig();

    if (root->parent())
      root = root->parent();

    rebuildTreemap(root);
  }
}

void KTreemapView::selectParent() {
  if (_selectedTile && _selectedTile->parentTile())
    selectTile(_selectedTile->parentTile());
}

bool KTreemapView::canZoomIn() const {
  if (!_selectedTile || !_rootTile)
    return false;

  if (_selectedTile == _rootTile)
    return false;

  KTreemapTile *newRootTile = _selectedTile;

  while (newRootTile->parentTile() != _rootTile &&
         newRootTile->parentTile()) // This should never happen, but who knows?
  {
    newRootTile = newRootTile->parentTile();
  }

  if (newRootTile) {
    KFileInfo *newRoot = newRootTile->orig();

    if (newRoot->isDir() || newRoot->isDotEntry())
      return true;
  }

  return false;
}

bool KTreemapView::canZoomOut() const {
  if (!_rootTile || !_tree->root())
    return false;

  return _rootTile->orig() != _tree->root();
}

bool KTreemapView::canSelectParent() const {
  return _selectedTile && _selectedTile->parentTile();
}

void KTreemapView::rebuildTreemap() {
  KFileInfo *root = 0;

  if (!_savedRootUrl.isEmpty()) {
    // qDebug() << "Restoring old treemap with root " << _savedRootUrl << endl;

    root = _tree->locate(_savedRootUrl, true); // node, findDotEntries
  }

  if (!root)
    root = _rootTile ? _rootTile->orig() : _tree->root();

  rebuildTreemap(root);
  _savedRootUrl = "";
}

void KTreemapView::rebuildTreemap(KFileInfo *newRoot) {
  QRect viewportRect(0, 0, this->width(), this->height());
  QRectF newSize = mapToScene(viewportRect).boundingRect();
  clear();
  if (newRoot) {
    QGraphicsScene *canv = new QGraphicsScene(this);
    canv->setSceneRect(newSize);
    _rootTile = new KTreemapTile(this,    // parentView
                                 0,       // parentTile
                                 newRoot, // orig
                                 newSize, KTreemapAuto);
    canv->addItem(_rootTile);
    setScene(canv);
  }

  // Synchronize selection with the tree
  updateSelection(_tree);
  emit treemapChanged();
}

void KTreemapView::deleteNotify(KFileInfo *) {
  if (_rootTile) {
    if (_rootTile->orig() != _tree->root()) {
      // If the user zoomed the treemap in, save the root's URL so the
      // current state can be restored upon the next rebuildTreemap()
      // call (which is triggered by the childDeleted() signal that the
      // tree emits after deleting is done).
      //
      // Intentionally using debugUrl() here rather than just url() so
      // the correct zoom can be restored even when a dot entry is the
      // current treemap root.

      _savedRootUrl = _rootTile->orig()->debugUrl();
    } else {
      // A shortcut for the most common case: No zoom. Simply use the
      // tree's root for the next treemap rebuild.

      _savedRootUrl = "";
    }
  } else {
    // Intentionally leaving _savedRootUrl alone: Otherwise multiple
    // deleteNotify() calls might cause a previously saved _savedRootUrl to
    // be unnecessarily deleted, thus the treemap couldn't be restored as
    // it was.
  }

  clear();
}

void KTreemapView::resizeEvent(QResizeEvent *event) {
  QGraphicsView::resizeEvent(event);
  _refreshTimer.start();
}

void KTreemapView::selectTile(KTreemapTile *tile, bool emitEvent) {
  // qDebug() << Q_FUNC_INFO << endl;

  KTreemapTile *oldSelection = _selectedTile;
  _selectedTile = tile;

  // Handle selection (highlight) rectangle

  if (_selectedTile) {
    if (!_selectionRect) {
      _selectionRect = new KTreemapSelectionRect(_highlightColor);
      scene()->addItem(_selectionRect);
    }
  }

  if (_selectionRect)
    _selectionRect->highlight(_selectedTile);

  if (emitEvent && oldSelection != _selectedTile) {
    std::vector<KFileInfo *> sel;
    if(_selectedTile != nullptr)
       sel.push_back(_selectedTile->orig());
    _tree->selectItems(sel);
  }
}

void KTreemapView::updateSelection(KDirTree * tree) {
  if(tree->selection().size() == 1)
    selectTile(findTile(tree->selection()[0]), false);
  else
    selectTile(nullptr, false);
}

KTreemapTile *KTreemapView::findTile(KFileInfo *node) {
  if (!node)
    return 0;

  foreach (QGraphicsItem *i, scene()->items()) {
    KTreemapTile *tile = dynamic_cast<KTreemapTile *>(i);
    if (tile && tile->orig() == node)
      return tile;
  }

  return 0;
}

QColor KTreemapView::tileColor(KFileInfo *file) {
  if (file) {
    if (file->isFile()) {
      // Find the filename extension: Everything after the first '.'
      QString ext = file->name().section('.', 1);

      while (!ext.isEmpty()) {
        QString lowerExt = ext.toLower();

        // Try case sensitive comparisions first

        if (ext == "~")
          return Qt::red;
        if (ext == "bak")
          return Qt::red;

        if (ext == "c")
          return Qt::blue;
        if (ext == "cpp")
          return Qt::blue;
        if (ext == "cc")
          return Qt::blue;
        if (ext == "h")
          return Qt::blue;
        if (ext == "hpp")
          return Qt::blue;
        if (ext == "el")
          return Qt::blue;

        if (ext == "o")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "lo")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "Po")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "al")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "moc.cpp")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "moc.cc")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "elc")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "la")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "a")
          return QColor(0xff, 0xa0, 0x00);
        if (ext == "rpm")
          return QColor(0xff, 0xa0, 0x00);

        // archives
        if (lowerExt == "tar.bz2")
          return Qt::green;
        if (lowerExt == "tar.gz")
          return Qt::green;
        if (lowerExt == "tar.xz")
          return Qt::green;
        if (lowerExt == "tgz")
          return Qt::green;
        if (lowerExt == "bz2")
          return Qt::green;
        if (lowerExt == "bz")
          return Qt::green;
        if (lowerExt == "gz")
          return Qt::green;

        if (lowerExt == "html")
          return Qt::blue;
        if (lowerExt == "htm")
          return Qt::blue;
        if (lowerExt == "txt")
          return Qt::blue;
        if (lowerExt == "doc")
          return Qt::blue;
        if (lowerExt == "php")
          return Qt::blue;
        if (lowerExt == "odt")
          return Qt::blue;
        if (lowerExt == "ods")
          return Qt::blue;

        if (lowerExt == "png")
          return Qt::cyan;
        if (lowerExt == "jpg")
          return Qt::cyan;
        if (lowerExt == "jpeg")
          return Qt::cyan;
        if (lowerExt == "gif")
          return Qt::cyan;
        if (lowerExt == "tif")
          return Qt::cyan;
        if (lowerExt == "tiff")
          return Qt::cyan;
        if (lowerExt == "bmp")
          return Qt::cyan;
        if (lowerExt == "xpm")
          return Qt::cyan;
        if (lowerExt == "tga")
          return Qt::cyan;
        if (lowerExt == "xcf")
          return Qt::cyan;

        if (lowerExt == "wav")
          return Qt::yellow;
        if (lowerExt == "mp3")
          return Qt::yellow;

        if (lowerExt == "avi")
          return QColor(0xa0, 0xff, 0x00);
        if (lowerExt == "mov")
          return QColor(0xa0, 0xff, 0x00);
        if (lowerExt == "mpg")
          return QColor(0xa0, 0xff, 0x00);
        if (lowerExt == "mpeg")
          return QColor(0xa0, 0xff, 0x00);
        if (lowerExt == "wmv")
          return QColor(0xa0, 0xff, 0x00);

        if (lowerExt == "pdf")
          return Qt::blue;
        if (lowerExt == "ps")
          return Qt::cyan;

        // databases
        if (lowerExt == "db")
          return QColor(0, 0, 127);
        if (lowerExt == "sqlite")
          return QColor(0, 0, 127);
        if (lowerExt == "sql")
          return QColor(0, 0, 127);

        // other
        if (lowerExt == "jar")
          return QColor(255, 255, 0);
        if (lowerExt == "cb")
          return QColor(190, 160, 100);
        if (lowerExt == "dat")
          return QColor(255, 255, 255);

        // Some DOS/Windows types

        if (lowerExt == "exe")
          return Qt::magenta;
        if (lowerExt == "com")
          return Qt::magenta;
        if (lowerExt == "dll")
          return QColor(0xff, 0xa0, 0x00);
        if (lowerExt == "zip")
          return Qt::green;
        if (lowerExt == "arj")
          return Qt::green;

        // No match so far? Try the next extension. Some files might have
        // more than one, e.g., "tar.bz2" - if there is no match for
        // "tar.bz2", there might be one for just "bz2".

        ext = ext.section('.', 1);
      }

      // Shared libs
      if (QRegExp("lib.*\\.so.*").exactMatch(file->name()))
        return QColor(0xff, 0xa0, 0x00);

      // Very special, but common: Core dumps
      if (file->name() == "core")
        return Qt::red;

      // Special case: Executables
      if ((file->mode() & S_IXUSR) == S_IXUSR)
        return Qt::magenta;
    } else // Directories
    {
      // TO DO
      return Qt::blue;
    }
  }

  return Qt::white;
}

KTreemapSelectionRect::KTreemapSelectionRect(const QColor &color) {
  setPen(QPen(color, 2));
  setZValue(1e10); // Higher than everything else
}

void KTreemapSelectionRect::highlight(KTreemapTile *tile) {
  if (tile) {
    setRect(tile->rect());

    if (!isVisible())
      show();
  } else {
    if (isVisible())
      hide();
  }
}

