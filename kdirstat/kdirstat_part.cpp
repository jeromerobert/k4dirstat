/*
 *   File name:	kdirstat_part.cpp
 *   Summary:	
 *   License:	LGPL - See file COPYING for details.
 *
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   $Id: kdirstat_part.cpp,v 1.1 2001/09/25 14:40:23 alexannika Exp $
 *
 */

#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kaction.h>

#include "kdirstat_part.h"

using namespace KDirStat;


KDirStatPart::KDirStatPart( QWidget * parent, const char * name ) : KParts::ReadWritePart( (QObject *)parent, name )
{
  KInstance * instance = new KInstance( "kdirstatpart" );
  setInstance( instance );

  treeview = new KDirTreeView(parent);

   setWidget( treeview );

    (void)new KAction( i18n( "Select All" ), 0, this,
		       SLOT( slotSelectAll() ), actionCollection(), "selectall" );
    setXMLFile( "kdirstat_part.rc" );

     setReadWrite( false );
}

void KDirStatPart::setReadWrite( bool rw )
{
  ReadWritePart::setReadWrite( rw );
}


bool KDirStatPart::openFile(){
  return 0;
}

bool KDirStatPart::saveFile(){
  return 0;
}
