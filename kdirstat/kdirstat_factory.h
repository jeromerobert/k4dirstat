/*
 *   File name:	kdirstat_factory.h
 *   Summary:	
 *   License:	LGPL - See file COPYING for details.
 *
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   $Id: kdirstat_factory.h,v 1.1 2001/09/25 14:40:23 alexannika Exp $
 *
 */


#ifndef KDirStat_Factory_h
#define KDirStat_Factory_h
 

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <klibloader.h>

class KInstance;
class KAboutData;

class KDirStatFactory : public KLibFactory
{
  Q_OBJECT
    public:
  KDirStatFactory( QObject * parent = 0, const char * name = 0 );
  ~KDirStatFactory();

   // reimplemented from KLibFactory
  virtual QObject * create( QObject * parent = 0, const char * name = 0,
			    const char * classname = "QObject",
			    const QStringList &args = QStringList());
   static KInstance * instance();

 private:
   static KInstance * s_instance;
   static KAboutData * s_about;
};









#endif // KDirStat_Factory_h
