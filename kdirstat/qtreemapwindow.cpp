/*
 *   File name:	qtreemapwindow.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: qtreemapwindow.cpp,v 1.2 2001/06/30 17:08:29 harry1701 Exp $
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
//#include <bits/mathcalls.h>

using namespace KDirStat;

QTreeMapWindow::QTreeMapWindow(  )  : QMainWindow() {
  options=new QTreeMapOptions();

  toolbar= new QToolBar(this);

  up_button=new QPushButton("Up",toolbar);
  zoom_in_button=new QPushButton("ZoomIn",toolbar);
  zoom_out_button=new QPushButton("ZoomOut",toolbar);
  dir_name_label=new QLabel("current dirname", toolbar);

  this->addToolBar(toolbar,"High!");

  menubar=this->menuBar();
  statusbar=this->statusBar();

  QPopupMenu *menu_file=new QPopupMenu(this);
  menu_file->insertTearOffHandle();
  menu_file->insertItem("&Save as Bitmap");
  
  menu_draw_mode=new QPopupMenu(this);
  makeRadioPopup(menu_draw_mode,QString("Files"),SLOT(selectDrawmode(int)),DM_FILES);
  makeRadioPopup(menu_draw_mode,QString("Dirs"),SLOT(selectDrawmode(int)),DM_DIRS);
  makeRadioPopup(menu_draw_mode,QString("Files & Dirs"),SLOT(selectDrawmode(int)),DM_BOTH);
  menu_draw_mode->setCheckable(TRUE);

  menu_paint_mode=new QPopupMenu(this);
  makeRadioPopup(menu_paint_mode,QString("Flat"), SLOT(selectShading(int)),PM_FLAT);
  makeRadioPopup(menu_paint_mode,QString("Sinus"), SLOT(selectShading(int)),PM_SIMPLE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("fast Sinus"), SLOT(selectShading(int)),PM_SQUARE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("Cone"), SLOT(selectShading(int)),PM_CONE_CUSHION);
  makeRadioPopup(menu_paint_mode,QString("Outline"), SLOT(selectShading(int)),PM_OUTLINE);
  makeRadioPopup(menu_paint_mode,QString("test Cushion"), SLOT(selectShading(int)),PM_CUSHION);
  menu_paint_mode->setCheckable(TRUE);

  menu_border_width=new QPopupMenu(this);
  for(int i=0; i<=10;i++){
    QString s;
    s.sprintf("%2d",i);
    makeRadioPopup(menu_border_width,s,SLOT(selectBorderWidth(int)),i);
  }
  menu_border_width->setCheckable(TRUE);

  menu_start_direction=new QPopupMenu(this);
  makeRadioPopup(menu_start_direction,QString("Horizontal"),SLOT(selectStartDirection(int)),HORIZONTAL);
  makeRadioPopup(menu_start_direction,QString("Vertikal"),SLOT(selectStartDirection(int)),VERTIKAL);

  menu_options=new QPopupMenu(this);
  menu_options->insertTearOffHandle();
  menu_options->insertItem("&Paint Mode",menu_paint_mode);
  menu_options->insertItem("&Draw Mode",menu_draw_mode);
  menu_options->insertItem("Border &Width",menu_border_width);
  menu_options->insertItem("Start &Direction",menu_start_direction);
  menu_options->insertItem("Draw &Text",this,SLOT(changeDrawText(int)));

  menu_file_id=menubar->insertItem("&File",menu_file);
  menu_options_id=menubar->insertItem("&Options",menu_options);
  

  scrollview=new QScrollView(this);
  graph_widget=new QTreeMapArea(scrollview->viewport());
  scrollview->addChild(graph_widget);

  this->setCentralWidget(scrollview);

  graph_widget->show();

  QObject::connect(up_button, SIGNAL(clicked()), graph_widget, SLOT(directoryUp()));
  QObject::connect(zoom_in_button, SIGNAL(clicked()), graph_widget, SLOT(zoomIn()));
  QObject::connect(zoom_out_button, SIGNAL(clicked()), graph_widget, SLOT(zoomOut()));

  QObject::connect(graph_widget,SIGNAL(highlighted(KDirInfo *)), this, SLOT(setStatusBar(KDirInfo *))  );
  QObject::connect(graph_widget,SIGNAL(changedDirectory(KDirInfo *)), this, SLOT(setDirectoryLabel(KDirInfo *))  );

  this->resize((options->paint_size_x)+50,(options->paint_size_y)+200);
  this->show();

  desktop=QApplication::desktop();
}

QTreeMapArea *QTreeMapWindow::getArea(){
  return graph_widget;
}

void QTreeMapWindow::makeRadioPopup(QPopupMenu *menu, const QString& title, const char *slot, const int param) {
  int id=menu->insertItem(title,this,slot);
  menu->setItemParameter(id,param);
  
}

void QTreeMapWindow::setStatusBar(KDirInfo *found){
  //  statusbar->message(found->debugUrl());

  KDirInfo *walk=found;
  QString mess=QString("");
  while(walk!=NULL){
    QString part;
    part.sprintf("%s/ %s",walk->name().latin1(),
		 graph_widget->tellUnit(walk->totalSize()).latin1());
    mess=part+"   "+mess;
    walk=(KDirInfo *)walk->parent();
  }

  statusbar->message(mess);
}

void QTreeMapWindow::setDirectoryLabel(KDirInfo *found){
  dir_name_label->setText(found->debugUrl());
}
void QTreeMapWindow::selectDrawmode(int id){
  options->draw_mode=id;
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
void QTreeMapWindow::selectStartDirection(int id){
  options->start_direction=id;
  redoOptions();
}
void QTreeMapWindow::changeDrawText(int id){
  NOT_USED(id);
  options->draw_text=!(options->draw_text);
  redoOptions();
}

void QTreeMapWindow::redoOptions(){
  graph_widget->setOptions(options);
}
