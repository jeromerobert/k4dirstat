/*
 *   File name:	kdirstat_part.h
 *   Summary:	
 *   License:	LGPL - See file COPYING for details.
 *
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   $Id: kdirstat_part.h,v 1.1 2001/09/25 14:40:23 alexannika Exp $
 *
 */


#ifndef KDirStat_Part_h
#define KDirStat_Part_h
 

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <kparts/part.h>

#include "kdirtreeview.h"

class KDirStatPart : public KParts::ReadWritePart
{
  Q_OBJECT
    public:
  KDirStatPart(QWidget * parent, const char * name = 0L );
  virtual ~KDirStatPart() {};

virtual void setReadWrite( bool rw );

 protected:
 virtual bool openFile();
  virtual bool saveFile();

 protected slots:
    //    void slotSelectAll();

 protected:
 KInstance *instance;

 KDirStat::KDirTreeView *treeview;
};



#endif // KDirStat_Part_h
