/*
 *   File name:	kdirtreemapwindow.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: 
 *
 */

#include <string.h>
#include <sys/errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <qtimer.h>
#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>
#include "kdirtree.h"
#include "kdirtreeview.h"
#include "kdirsaver.h"
#include "qtreemap.h"
#include <qmainwindow.h>
#include "qtreemapwindow.h"
#include "kdirtreemap.h"
#include "kdirtreemapwindow.h"

using namespace KDirStat;

KDirTreeMapWindow::KDirTreeMapWindow(  )  : QTreeMapWindow() {
}

QTreeMapArea *KDirTreeMapWindow::makeTreeMapWidget(QWidget *parent){
  return new KDirTreeMapArea(parent);
}
