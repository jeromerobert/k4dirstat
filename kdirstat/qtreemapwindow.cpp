/*
 *   File name:	qtreemapwindow.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-07-11
 *
 *   $Id: qtreemapwindow.cpp,v 1.11 2001/07/18 03:09:39 alexannika Exp $
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

using namespace KDirStat;

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
  menu_file->insertItem("&Save as Bitmap");

  menu_draw_mode=new QPopupMenu(this);
#if 0
  makeRadioPopup(menu_draw_mode,QString("Files"),SLOT(selectDrawmode(int)),DM_FILES);
  makeRadioPopup(menu_draw_mode,QString("Dirs"),SLOT(selectDrawmode(int)),DM_DIRS);
  makeRadioPopup(menu_draw_mode,QString("Files & Dirs"),SLOT(selectDrawmode(int)),DM_BOTH);
  menu_draw_mode->setCheckable(TRUE);
#endif

  makeBrainPopup(menu_draw_mode,QString("Files"),SLOT(selectDrawmode(int)),DM_FILES,"drawmode","files_only");
  makeBrainPopup(menu_draw_mode,QString("Dirs"),SLOT(selectDrawmode(int)),DM_DIRS,"drawmode","dirs_only");
  makeBrainPopup(menu_draw_mode,QString("Files & Dirs"),SLOT(selectDrawmode(int)),DM_BOTH,"drawmode","both");
  menu_draw_mode->setCheckable(TRUE);



  menu_paint_mode=new QPopupMenu(this);
  makeRadioPopup(menu_paint_mode,QString("Flat"), SLOT(selectShading(int)),PM_FLAT);
  makeRadioPopup(menu_paint_mode,QString("Images"), SLOT(selectShading(int)),PM_IMAGES);
  makeRadioPopup(menu_paint_mode,QString("Sinus"), SLOT(selectShading(int)),PM_SIMPLE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("Sinus 2"), SLOT(selectShading(int)),PM_WAVE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("fast Bumb"), SLOT(selectShading(int)),PM_SQUARE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("Cone"), SLOT(selectShading(int)),PM_CONE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("Outline"), SLOT(selectShading(int)),PM_OUTLINE);
  makeRadioPopup(menu_paint_mode,QString("wave2 Cushion"), SLOT(selectShading(int)),PM_WAVE2_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("test CTM Cushion"), SLOT(selectShading(int)),PM_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("hierarch. Sinus Cushion"), SLOT(selectShading(int)),PM_HIERARCH_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("hierarch. Sinus Cushion 2"), SLOT(selectShading(int)),PM_HIERARCH2_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("hierarch. Dist. Cushion"), SLOT(selectShading(int)),PM_HIERARCH3_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("hierarch. Dist. Cushion 2"), SLOT(selectShading(int)),PM_HIERARCH4_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("hierarch. pyramid"), SLOT(selectShading(int)),PM_HIERARCH_PYRAMID);
  makeRadioPopup(menu_paint_mode,QString("hierarch. dist. pyramid"), SLOT(selectShading(int)),PM_HIERARCH_DIST_PYRAMID);
  makeRadioPopup(menu_paint_mode,QString("hierarch. dist. sin pyramid"), SLOT(selectShading(int)),PM_HIERARCH_DIST_SIN_PYRAMID);
  makeRadioPopup(menu_paint_mode,QString("hierarch.5 Cushion"), SLOT(selectShading(int)),PM_HIERARCH5_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("hierarch. Debug Cushion"), SLOT(selectShading(int)),PM_HIERARCH_TEST_CUSHION);
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
  makeRadioPopup(menu_dont_draw,"off",SLOT(selectDontDrawOption(int)),-1);
  for(int i=0; i<=10;i++){
    QString s;
    s.sprintf("%2d",i);
    makeRadioPopup(menu_dont_draw,s, SLOT(selectDontDrawOption(int)),i);
  }
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

  menu_options=new QPopupMenu(this);
  menu_options->insertTearOffHandle();
  menu_options->insertItem("&Shadings",menu_paint_mode);
  menu_options->insertItem("&Draw Mode",menu_draw_mode);
  menu_options->insertItem("Border &Width (any)",menu_border_width);
  menu_options->insertItem("Border &Step (node)",menu_border_step);
  menu_options->insertItem("Start &Direction",menu_start_direction);
  menu_options->insertItem("Dont Draw if smaller",menu_dont_draw);
  menu_options->insertItem("Hierarch. Cushion Factor",menu_hfactor);
  menu_options->insertItem("Test Cushion Factor h=",menu_shfactor);
  menu_options->insertItem("Test Cushion Factor f=",menu_sffactor);
  menu_options->insertItem("Color Scheme",menu_colorscheme);
  menu_options->insertItem("Draw &Text",this,SLOT(changeDrawText(int)));
  menu_options->insertItem("squarify Treemaps",this,SLOT(changeSquarifyTreemaps(int)));
  menu_options->insertItem("show inode space",this,SLOT(changeShowInodeSpace(int)));
  menu_options->insertItem("dynamic shading",this,SLOT(changeDynamicShading(int)));

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

  sleep(1);
  printf("CONNECTS\n");
  QObject::connect(find_entry,SIGNAL(returnPressed()), this, SLOT(findMatch()));

  QObject::connect(up_button, SIGNAL(clicked()), qtm_area, SLOT(directoryUp()));
  QObject::connect(zoom_in_button, SIGNAL(clicked()), qtm_area, SLOT(zoomIn()));
  QObject::connect(zoom_out_button, SIGNAL(clicked()), qtm_area , SLOT(zoomOut()));


  QObject::connect(qtm_area ,SIGNAL(highlighted(Object *)), this, SLOT(setStatusBar(Object *))  );
  QObject::connect(qtm_area ,SIGNAL(changedDirectory(Object *)), this, SLOT(setDirectoryLabel(Object *))  );

  printf("CONNECTSEND\n");

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
    part.sprintf("%s/ %s",graph_widget->shortName(walk).latin1(),
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
   for(int i=0;i<brainlist->count();i++){
     OptionsBrain *brain=brainlist->at(i);
     if(brain->groupname==gname &&
	brain->parameter==param){
       printf("set group %s option %s param %d\n",gname.latin1(),brain->optionname.latin1(),brain->parameter);

       brain->popup->setItemChecked(brain->menuid,TRUE);
     }
     else if(brain->groupname==gname){
       brain->popup->setItemChecked(brain->menuid,FALSE);
     }
   }
}

 int QTreeMapWindow::getBrainParamByName(QString gname,QString oname){
   for(int i=0;i<brainlist->count();i++){
     if(brainlist->at(i)->groupname==gname &&
	brainlist->at(i)->optionname==oname){
       printf("group %s option %s param %d\n",gname.latin1(),oname.latin1(),brainlist->at(i)->parameter);
       return brainlist->at(i)->parameter;
     }
   }
   printf("REAL BIG BRAIN FAULT! %s %s\n",gname.latin1(),oname.latin1());
   return 0; //urks
 }

OptionsBrain::OptionsBrain(QPopupMenu *menu,QString group_name,QString option_name,int group_id,int menu_id,int param){
  popup=menu;
  groupname=group_name;
  optionname=option_name;
  groupid=group_id;
  menuid=menu_id;
  parameter=param;
}
