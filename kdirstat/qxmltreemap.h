/*
 *   File name:	qxmltreemap.h
 *   Summary:	
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: 
 *
 */

#ifndef QXmlTreeMap_h
#define QXmlTreeMap_h


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <qqueue.h>
#include <kfileitem.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qmainwindow.h>
#include <qpen.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qbutton.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qscrollview.h>
#include "qtreemap.h"
#include <qdom.h>

#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif

class Object;

// Open a new name space since KDE's name space is pretty much cluttered
// already - all names that would even remotely match are already used up,
// yet the resprective classes don't quite fit the purposes required here.

//namespace KDirStat
//{

  class QXmlTreeMapArea : public QTreeMapArea {
    Q_OBJECT

  public:
    //KTreeMap(QWidget *parent=0);
    QXmlTreeMapArea(QWidget *parent=0);
    //~QTreeMapArea();

    void printXmlInfo(Object *node);
    QDomElement findElement(QDomNode n);

    // reimplemented abstract functions

    void dirChange(Object *node);

    QString shortName(Object *node);
    QString fullName(Object *node);
    Object *firstChild(Object *node);
    int   totalSize(Object *node);
    int   totalItems(Object *node);
    int   thisDirItems(Object *node);
    //    int   areaSize(Object *node);
    bool isLeaf(Object *node);
    bool isNode(Object *node);
    bool isSameLevelChild(Object *node);
    Object *nextChild(Object *node);
    Object *sameLevelChild(Object *node);
    QString tellUnit(int size);
    Object *parentNode(Object *node);

    public slots:

      void directoryUp();
    void zoomIn();
    void zoomOut();
    void saveAsBitmap();
    void saveAsXML();

  signals:

    void highlighted(Object *high);
    void changedDirectory(Object *high);

  };

//} // namespace


#endif // ifndef QXmlTreeMap_h


// EOF
