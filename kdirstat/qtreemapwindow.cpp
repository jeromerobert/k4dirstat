/*
 *   File name:	qtreemapwindow.cpp
 *   Summary:	
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-07-11
 *
 *   $Id: qtreemapwindow.cpp,v 1.16 2001/08/11 23:55:35 alexannika Exp $
 *
 */

#include <string.h>
#include <sys/errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <qtimer.h>
#include <qapplication.h>
#include "qtreemap.h"
#include <qmainwindow.h>
#include "qtreemapwindow.h"
//#include "kdirtreemap.h"

//using namespace KDirStat;

QTreeMapWindow::QTreeMapWindow(  )  : QMainWindow() {

  options=new QTreeMapOptions();

  //makeWidgets();
}

void QTreeMapWindow::makeWidgets(){

  brainlist=new QList<OptionsBrain>();

  toolbar= new QToolBar(this);

  up_button=new QPushButton("Up",toolbar);
  zoom_in_button=new QPushButton("ZoomIn",toolbar);
  zoom_out_button=new QPushButton("ZoomOut",toolbar);
  find_entry=new QLineEdit(toolbar);
  dir_name_label=new QLabel("current dirname", toolbar);

  this->addToolBar(toolbar,"High!");

  menubar=this->menuBar();
  statusbar=this->statusBar();

  QPopupMenu *menu_file=new QPopupMenu(this);
  menu_file->insertTearOffHandle();
  menu_file->insertItem("&Save as Bitmap",this,SLOT(saveAsBitmap()));
  menu_file->insertItem("Save as XML file",this,SLOT(saveAsXML()));
  menu_file->insertItem("Load from XML file",this,SLOT(loadFromXML()));
  menu_file->insertItem("Save as HypView file",this,SLOT(saveAsHypView()));
  menu_file->insertItem("Call HypView",this,SLOT(callHypView()));

  menu_draw_mode=new QPopupMenu(this);

  makeBrainPopup(menu_draw_mode,QString("Files"),SLOT(selectDrawmode(int)),DM_FILES,"drawmode","files_only");
  makeBrainPopup(menu_draw_mode,QString("Dirs"),SLOT(selectDrawmode(int)),DM_DIRS,"drawmode","dirs_only");
  makeBrainPopup(menu_draw_mode,QString("Files & Dirs"),SLOT(selectDrawmode(int)),DM_BOTH,"drawmode","both");
  menu_draw_mode->setCheckable(TRUE);



  QString shading=QString("shading");
  menu_paint_mode=new QPopupMenu(this);
  makeBrainPopup(menu_paint_mode,QString("Flat"), SLOT(selectShading(int)),PM_FLAT,shading,"flat");
  makeBrainPopup(menu_paint_mode,QString("Images"), SLOT(selectShading(int)),PM_IMAGES,shading,"images");
  makeBrainPopup(menu_paint_mode,QString("Sinus"), SLOT(selectShading(int)),PM_SIMPLE_CUSHION,shading,"sinus");
  makeBrainPopup(menu_paint_mode,QString("Sinus 2"), SLOT(selectShading(int)),PM_WAVE_CUSHION,shading,"sinus2");
  makeBrainPopup(menu_paint_mode,QString("fast Bumb"), SLOT(selectShading(int)),PM_SQUARE_CUSHION,shading,"bump");
  makeBrainPopup(menu_paint_mode,QString("Cone"), SLOT(selectShading(int)),PM_CONE_CUSHION,shading,"cone");
  makeBrainPopup(menu_paint_mode,QString("Outline"), SLOT(selectShading(int)),PM_OUTLINE,shading,"outline");
  makeBrainPopup(menu_paint_mode,QString("wave2 Cushion"), SLOT(selectShading(int)),PM_WAVE2_CUSHION,shading,"wave2");
  makeBrainPopup(menu_paint_mode,QString("test CTM Cushion"), SLOT(selectShading(int)),PM_CUSHION,shading,"ctm_cushion");
  makeBrainPopup(menu_paint_mode,QString("hierarch. Sinus Cushion"), SLOT(selectShading(int)),PM_HIERARCH_CUSHION,shading,"hierarch_sinus");
  makeBrainPopup(menu_paint_mode,QString("hierarch. Sinus Cushion 2"), SLOT(selectShading(int)),PM_HIERARCH2_CUSHION,shading,"hierarch_sinus2");
  makeBrainPopup(menu_paint_mode,QString("hierarch. Dist. Cushion"), SLOT(selectShading(int)),PM_HIERARCH3_CUSHION,shading,"hierarch_dist");
  makeBrainPopup(menu_paint_mode,QString("hierarch. Dist. Cushion 2"), SLOT(selectShading(int)),PM_HIERARCH4_CUSHION,shading,"hierarch_dist2");
  makeBrainPopup(menu_paint_mode,QString("hierarch. pyramid"), SLOT(selectShading(int)),PM_HIERARCH_PYRAMID,shading,"hierarch_pyramid");
  makeBrainPopup(menu_paint_mode,QString("hierarch. dist. pyramid"), SLOT(selectShading(int)),PM_HIERARCH_DIST_PYRAMID,shading,"hierarch_dist_pyramid");
  makeBrainPopup(menu_paint_mode,QString("hierarch. dist. sin pyramid"), SLOT(selectShading(int)),PM_HIERARCH_DIST_SIN_PYRAMID,shading,"hierarch_dist_sin_pyramid");
  makeBrainPopup(menu_paint_mode,QString("hierarch.5 Cushion"), SLOT(selectShading(int)),PM_HIERARCH5_CUSHION,shading,"hierarch5");
  makeBrainPopup(menu_paint_mode,QString("hierarch. Debug Cushion"), SLOT(selectShading(int)),PM_HIERARCH_TEST_CUSHION,shading,"hierarch_debug");
  menu_paint_mode->setCheckable(TRUE);

  menu_border_width=new QPopupMenu(this);
  for(int i=0; i<=10;i++){
    QString s;
    s.sprintf("%2d",i);
    makeRadioPopup(menu_border_width,s, SLOT(selectBorderWidth(int)),i);
  }
  menu_border_width->setCheckable(TRUE);

  menu_border_step=new QPopupMenu(this);
  for(int i=0; i<=10;i++){
    QString s;
    s.sprintf("%2d",i);
    makeRadioPopup(menu_border_step,s, SLOT(selectBorderStep(int)),i);
  }
  menu_border_width->setCheckable(TRUE);

  menu_dont_draw=new QPopupMenu(this);
#if 1
  makeRadioPopup(menu_dont_draw,"off",SLOT(selectDontDrawOption(int)),-1);
  for(int i=0; i<=10;i++){
    QString s;
    s.sprintf("%2d",i);
    makeRadioPopup(menu_dont_draw,s, SLOT(selectDontDrawOption(int)),i);
  }
#else

  makeBrainPopup(menu_dont_draw,"off",SLOT(selectDontDrawOption(int)),-1,"dontdraw","off");
  for(int i=0; i<=10;i++){
    QString s;
    s.sprintf("%2d",i);
    makeBrainPopup(menu_dont_draw,s, SLOT(selectDontDrawOption(int)),i,"dontdraw",s2);
  }
#endif
  menu_dont_draw->setCheckable(TRUE);

  menu_hfactor=new QPopupMenu(this);
  for(int i=-100; i<=100;i+=10){
    QString s;
    s.sprintf("%2d %%",i);
    makeRadioPopup(menu_hfactor,s, SLOT(selectHFactor(int)),i);
  }
  menu_hfactor->setCheckable(TRUE);

  menu_shfactor=new QPopupMenu(this);
  for(int i=-100; i<=100;i+=10){
    QString s;
    s.sprintf("%2d %%",i);
    makeRadioPopup(menu_shfactor,s, SLOT(selectSHFactor(int)),i);
  }
  menu_shfactor->setCheckable(TRUE);

  menu_sffactor=new QPopupMenu(this);
  for(int i=-100; i<=100;i+=10){
    QString s;
    s.sprintf("%2d %%",i);
    makeRadioPopup(menu_sffactor,s, SLOT(selectSFFactor(int)),i);
  }
  menu_sffactor->setCheckable(TRUE);

  menu_start_direction=new QPopupMenu(this);
  makeRadioPopup(menu_start_direction,QString("Horizontal"),SLOT(selectStartDirection(int)),HORIZONTAL);
  makeRadioPopup(menu_start_direction,QString("Vertikal"),SLOT(selectStartDirection(int)),VERTIKAL);

  menu_colorscheme=new QPopupMenu(this);
  makeRadioPopup(menu_colorscheme,QString("cyclic"),SLOT(selectColorScheme(int)),CS_CYCLIC);
  makeRadioPopup(menu_colorscheme,QString("regexp"),SLOT(selectColorScheme(int)),CS_REGEXP);
  makeRadioPopup(menu_colorscheme,QString("monochrome"),SLOT(selectColorScheme(int)),CS_MONO);

#ifdef HAVE_PIEMAP
  ribbonmap_options=new QPopupMenu(this);
  //  ribbonmap_options->insertItem("Draw RibbonTree",this,SLOT(
 makeBrainPopup(ribbonmap_options,QString("ribbonmap"), SLOT(toggleRibbonOptions(int)),RIBBON_RIBBONMAP,"ribbonmap","ribbonmap");
 makeBrainPopup(ribbonmap_options,QString("Draw RibbonTree"), SLOT(toggleRibbonOptions(int)),RIBBON_DRAWTREE,"ribbonmap","drawtree");
 makeBrainPopup(ribbonmap_options,QString("Draw RibbonPie"), SLOT(toggleRibbonOptions(int)),RIBBON_DRAWPIE,"ribbonmap","drawpie");
 makeBrainPopup(ribbonmap_options,QString("use totalitems as size"), SLOT(toggleRibbonOptions(int)),RIBBON_USE_TOTALITEMS,"ribbonmap","use_totalitems");
#endif

  menu_options=new QPopupMenu(this);
  menu_options->insertTearOffHandle();
  menu_options->insertItem("&Shadings",menu_paint_mode);
  menu_options->insertItem("&Draw Mode",menu_draw_mode);
  menu_options->insertItem("Start &Direction",menu_start_direction);
  menu_options->insertItem("Dont Draw if smaller",menu_dont_draw);
#ifdef EXPERIMENTAL
  menu_options->insertItem("Border &Width (any)",menu_border_width);
  menu_options->insertItem("Border &Step (node)",menu_border_step);
  menu_options->insertItem("Hierarch. Cushion Factor",menu_hfactor);
  menu_options->insertItem("Test Cushion Factor h=",menu_shfactor);
  menu_options->insertItem("Test Cushion Factor f=",menu_sffactor);
  menu_options->insertItem("show inode space",this,SLOT(changeShowInodeSpace(int)));
  menu_options->insertItem("dynamic shading",this,SLOT(changeDynamicShading(int)));
#endif
  menu_options->insertItem("Draw &Text",this,SLOT(changeDrawText(int)));
  menu_options->insertItem("Color Scheme",menu_colorscheme);
  menu_options->insertItem("squarify Treemaps",this,SLOT(changeSquarifyTreemaps(int)));
#ifdef HAVE_PIEMAP
  menu_options->insertItem("RibbonMap Options",ribbonmap_options);
#endif

  menu_file_id=menubar->insertItem("&File",menu_file);
  menu_options_id=menubar->insertItem("&Options",menu_options);
  

  scrollview=new QScrollView(this);

  //graph_widget=new QTreeMapArea(scrollview->viewport());
 //graph_widget=new KDirTreeMapArea(scrollview->viewport());
    graph_widget=makeTreeMapWidget(scrollview->viewport());

  scrollview->addChild(graph_widget);

  this->setCentralWidget(scrollview);

  graph_widget->show();

  graph_widget->setOptions(options);

  QTreeMapArea *qtm_area=(QTreeMapArea *)graph_widget;


  QObject::connect(find_entry,SIGNAL(returnPressed()), this, SLOT(findMatch()));

  QObject::connect(up_button, SIGNAL(clicked()), qtm_area, SLOT(directoryUp()));
  QObject::connect(zoom_in_button, SIGNAL(clicked()), qtm_area, SLOT(zoomIn()));
  QObject::connect(zoom_out_button, SIGNAL(clicked()), qtm_area , SLOT(zoomOut()));


  QObject::connect(qtm_area ,SIGNAL(highlighted(Object *)), this, SLOT(setStatusBar(Object *))  );
  QObject::connect(qtm_area ,SIGNAL(changedDirectory(Object *)), this, SLOT(setDirectoryLabel(Object *))  );


  this->resize((options->paint_size_x)+50,(options->paint_size_y)+200);
  this->show();


  desktop=QApplication::desktop();
}

QTreeMapArea *QTreeMapWindow::getArea(){
  return graph_widget;
}

void QTreeMapWindow::makeBrainPopup(QPopupMenu *menu,const QString& title, const char *slot,const int param,const QString& groupname,const QString& optionname){

   int id=menu->insertItem(title,this,slot);
   menu->setItemParameter(id,param);
 
   brainlist->append(new OptionsBrain(menu,groupname,optionname,0,id,param) );
  
}

void QTreeMapWindow::makeRadioPopup(QPopupMenu *menu,const QString& title, const char *slot,const int param){
  int id=menu->insertItem(title,this,slot);
  menu->setItemParameter(id,param);
  
}

void QTreeMapWindow::findMatch(){
  QString find=find_entry->text();

  graph_widget->findMatch(find);
}


void QTreeMapWindow::setStatusBar(Object *found){
  //  statusbar->message(found->debugUrl());

  Object *walk=found;
  QString mess=QString("");
  while(walk!=NULL){
    QString part;
    part.sprintf("%s%s %s",graph_widget->shortName(walk).latin1(),
		 options->path_separator.latin1(),
		 graph_widget->tellUnit(  graph_widget->totalSize(walk)).latin1());
    mess=part+"   "+mess;
    walk=graph_widget->parentNode(walk);
  }

  statusbar->message(mess);
}

void QTreeMapWindow::setDirectoryLabel(Object *found){
  dir_name_label->setText(graph_widget->fullName(found));
}
void QTreeMapWindow::selectDrawmode(int id){
  options->draw_mode=id;
  setBrainCheckMark(options,id,"drawmode");
  redoOptions();
}
void QTreeMapWindow::selectShading(int id){
  options->paintmode=id;
  setBrainCheckMark(options,id,"shading");
  redoOptions();
}
void QTreeMapWindow::selectBorderWidth(int id){
  options->step_width=id;
  redoOptions();
}
void QTreeMapWindow::selectBorderStep(int id){
  options->border_step=id;
  redoOptions();
}
void QTreeMapWindow::selectStartDirection(int id){
  options->start_direction=id;
  redoOptions();
}
void QTreeMapWindow::selectColorScheme(int id){
  options->color_scheme=id;
  redoOptions();
}
void QTreeMapWindow::selectDontDrawOption(int id){
  options->dont_draw_xyd=id;
  redoOptions();
}
void QTreeMapWindow::changeDrawText(int id){
  NOT_USED(id);
  options->draw_text=!(options->draw_text);
  redoOptions();
}
void QTreeMapWindow::changeDynamicShading(int id){
  NOT_USED(id);
  options->dynamic_shading=!(options->dynamic_shading);
  redoOptions();
}
void QTreeMapWindow::changeSquarifyTreemaps(int id){
  NOT_USED(id);
  options->squarify=!(options->squarify);
  redoOptions();
}
void QTreeMapWindow::changeShowInodeSpace(int id){
  NOT_USED(id);
  options->show_inodes=!(options->show_inodes);
  redoOptions();
}
void QTreeMapWindow::selectHFactor(int id){
  NOT_USED(id);
  options->hc_factor=((float)id)/100.0;
  redoOptions();
}
void QTreeMapWindow::selectSHFactor(int id){
  NOT_USED(id);
  options->sequoia_h=((float)id)/100.0;
  redoOptions();
}
void QTreeMapWindow::selectSFFactor(int id){
  NOT_USED(id);
  options->sequoia_f=((float)id)/100.0;
  redoOptions();
}
void QTreeMapWindow::toggleRibbonOptions(int id){
  NOT_USED(id);
#ifdef HAVE_PIEMAP
  NOT_USED(id);
  if(id==RIBBON_DRAWTREE){
    options->draw_hyper_lines=!options->draw_hyper_lines;
  }
  else if(id==RIBBON_DRAWPIE){
    options->draw_pie_lines=!options->draw_pie_lines;
  }
  else if(id==RIBBON_USE_TOTALITEMS){
    if(options->area_is==AREA_IS_TOTALSIZE){
      options->area_is=AREA_IS_TOTALITEMS;
    }
    else if(options->area_is==AREA_IS_TOTALITEMS){
      options->area_is=AREA_IS_TOTALSIZE;
    }
  }
  else if(id==RIBBON_RIBBONMAP){
    options->piemap=!options->piemap;
  }

  redoOptions();
#endif
}

void QTreeMapWindow::redoOptions(){
  graph_widget->setOptions(options);
}
#if 0
int QTreeMapWindow::makeBrainMenuOption(QString& gname,QString& defaultstr){
  int param;
  QString modestr=config->readEntry(gname,defaultstr);
  param=getBrainParamByName("drawmode",modestr);
  setBrainCheckMark(NULL,param,gname);

  return param;
}
#endif

void QTreeMapWindow::setBrainCheckMark(QTreeMapOptions *opt,int param,QString gname){
  NOT_USED(opt);
   for(uint i=0;i<brainlist->count();i++){
     OptionsBrain *brain=brainlist->at(i);
     if(brain->groupname==gname &&
	brain->parameter==param){
       //       printf("set group %s option %s param %d\n",gname.latin1(),brain->optionname.latin1(),brain->parameter);

       brain->popup->setItemChecked(brain->menuid,TRUE);
     }
     else if(brain->groupname==gname){
       brain->popup->setItemChecked(brain->menuid,FALSE);
     }
   }
}

 int QTreeMapWindow::getBrainParamByName(QString gname,QString oname){
   for(uint i=0;i<brainlist->count();i++){
     if(brainlist->at(i)->groupname==gname &&
	brainlist->at(i)->optionname==oname){
       //printf("group %s option %s param %d\n",gname.latin1(),oname.latin1(),brainlist->at(i)->parameter);
       return brainlist->at(i)->parameter;
     }
   }
#ifdef EXPERIMENTAL
   printf("REAL BIG BRAIN FAULT! %s %s\n",gname.latin1(),oname.latin1());
#endif
   return 0; //urks
 }

void QTreeMapWindow::saveAsBitmap(){
  //printf("CALLBACK KDirTreeMapArea\n");
  graph_widget->saveAsBitmap();

}

void QTreeMapWindow::saveAsXML(){
  //printf("CALLBACK KDirTreeMapArea\n");
  graph_widget->saveAsXML();

}
void QTreeMapWindow::callHypView(){
  //printf("CALLBACK KDirTreeMapArea\n");
  graph_widget->callHypView();

}
void QTreeMapWindow::saveAsHypView(){
  //printf("CALLBACK KDirTreeMapArea\n");
  graph_widget->saveAsHypView();

}

void QTreeMapWindow::loadFromXML(){
  graph_widget->loadFromXML();

}

OptionsBrain::OptionsBrain(QPopupMenu *menu,QString group_name,QString option_name,int group_id,int menu_id,int param){
  popup=menu;
  groupname=group_name;
  optionname=option_name;
  groupid=group_id;
  menuid=menu_id;
  parameter=param;
}
