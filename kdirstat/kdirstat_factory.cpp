/*
 *   File name:	kdirstat_factory.cpp
 *   Summary:	
 *   License:	LGPL - See file COPYING for details.
 *
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   $Id: kdirstat_factory.cpp,v 1.1 2001/09/25 14:40:23 alexannika Exp $
 *
 */

#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "kdirstat_factory.h"

#include <klocale.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "kdirstat_part.h"

extern "C"
{
  void* init_libkdirstat()
  {
    return new KDirStatFactory;
  }
};

KInstance* KDirStatFactory::s_instance = 0L;
KAboutData* KDirStatFactory::s_about = 0L;

 KDirStatFactory::KDirStatFactory( QObject* parent, const char* name )
    : KLibFactory( parent, name )
 {
 }



KDirStatFactory::~KDirStatFactory()
{
  delete s_instance;
  s_instance = 0;
  //  delete s_about;
}

QObject* KDirStatFactory::create( QObject* parent, const char* name, const char*classname, const QStringList & )
{
  KDirStatPart *part=new KDirStatPart((QWidget*) parent, name );

  if (QCString(classname) == "KParts::ReadOnlyPart")
    part->setReadWrite(false);

    // otherwise, it has to be readwrite
    else if (QCString(classname) != "KParts::ReadWritePart")
    {
      kdError() << "classname isn't ReadOnlyPart nor ReadWritePart !" << endl;
      return 0L;
    }
 
  emit objectCreated( part );
  return part;
 
}

KInstance *KDirStatFactory::instance()
{
  if ( !s_instance )
    s_instance = new KInstance( "kdirstatpart");
  return s_instance;
}

#if 0
extern "C"
{
  void* init_libwebarchiverplugin()
  {
    KGlobal::locale()->insertCatalogue("webarchiver");
    return new KDirStatFactory;
  }
}

#endif

#include <kdirstat_factory.moc>

