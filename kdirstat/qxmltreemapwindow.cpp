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
#include "qxmltreemap.h"
#include "qxmltreemapwindow.h"

using namespace KDirStat;

QXmlTreeMapWindow::QXmlTreeMapWindow(  )  : QTreeMapWindow() {
  //setConfig();
}

QTreeMapArea *QXmlTreeMapWindow::makeTreeMapWidget(QWidget *parent){
  return new QXmlTreeMapArea(parent);
}

#if 1

int QXmlTreeMapWindow::makeBrainMenuOption(QString gname,QString defaultstr){
  int param;
  QString modestr=config->readEntry(gname,defaultstr);
  param=getBrainParamByName("drawmode",modestr);
  setBrainCheckMark(NULL,param,gname);

  return param;
}

void QXmlTreeMapWindow::setConfig(){
  config=KGlobal::config();

  config->setGroup("Treemap-Options");

  //QTreeMapOptions *opt=new QTreeMapOptions();
  QTreeMapOptions *opt=options;
#if 1
  opt->squarify=config->readBoolEntry("squarify",opt->squarify);
  
  //  opt->paintmode=config->readUnsignedNumEntry("shading",opt->paintmode);

  opt->paintmode=makeBrainMenuOption("shading","flat");

  //  opt->draw_mode=config->readUnsignedNumEntry("drawmode",opt->draw_mode);

  //QString modestr=config->readEntry("drawmode","both");
  //opt->draw_mode=getBrainParamByName("drawmode",modestr);
  opt->draw_mode=makeBrainMenuOption(QString("drawmode"),QString("both"));

  opt->mono_color=config->readColorEntry("monocolor",&opt->mono_color);

  QStringList group_list=config->groupList();

  QRegExp reg=QRegExp("Treemap-Color-");

  options->scheme_list=new QList<QTMcolorScheme>();

  for(int i=0;i<group_list.count();i++){
      printf("config0: %s\n",group_list[i].latin1());
    if(reg.match(group_list[i])!=-1){
      printf("config: %s\n",group_list[i].latin1());
      config->setGroup(group_list[i]);
      
      QTMcolorScheme *scheme=new QTMcolorScheme();

      scheme->schemeName=group_list[i];
      scheme->type=config->readUnsignedNumEntry("type",CST_REGEXP);
      scheme->patternlist=config->readListEntry("pattern",',');

      bool use_wildcards=TRUE;
	if(scheme->type==CST_REGEXP){
	  use_wildcards=FALSE;
	}

      scheme->regexplist=new QList<QRegExp>;
      for(int p=0;p<scheme->patternlist.count();p++){
#if 0
	  QString regstr=QString(scheme->patternlist[p]);
	  regstr=regstr.remove(0,1);
	  regstr=regstr.remove(regstr.length()-1,1);

	    printf("pattern +%s+ +%s+ on  %d %d %d\n",scheme->patternlist[p].latin1(),regstr.latin1(),scheme->color.red(),scheme->color.green(),scheme->color.blue());
#endif
	    printf("pattern +%s+\n",scheme->patternlist[p].latin1());
	QRegExp *regexp=new QRegExp(scheme->patternlist[p],FALSE,use_wildcards);
	scheme->regexplist->append(regexp);
      }
      QColor col=QColor(255,0,255);
      scheme->color=QColor(config->readColorEntry("color",&col));

	    printf("%d %d %d\n",scheme->color.red(),scheme->color.green(),scheme->color.blue());

      scheme->comment=config->readEntry("comment","no comment");
      
      options->scheme_list->append(scheme);
    }
  }


#endif
  //getArea()->setOptions(opt);

  //  delete opt;
}

#endif
