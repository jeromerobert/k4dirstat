/*
 *   File name:	qxmltreemapwindow.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: 
 *
 */

#ifndef QXmlTreeMapWindow_h
#define QXmlTreeMapWindow_h


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
#include "kdirtree.h"
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
#include <kconfig.h>
#include "qtreemap.h"
#include "qxmltreemap.h"

#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif

// Open a new name space since KDE's name space is pretty much cluttered
// already - all names that would even remotely match are already used up,
// yet the resprective classes don't quite fit the purposes required here.

namespace KDirStat
{

  class QTreeMapArea;
  class QTreeMapOptions;
  class QTreeMapWindow;


  class QXmlTreeMapWindow : public QTreeMapWindow {
  public:

    QXmlTreeMapWindow();

    QTreeMapArea *makeTreeMapWidget(QWidget *parent);
int makeBrainMenuOption(QString gname,QString defaultstr);

    void setConfig();

  private:
    
    KConfig *config;
  };
} // namespace

#endif



