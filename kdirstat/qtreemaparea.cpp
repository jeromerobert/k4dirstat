/*
 *   File name:	qtreemaparea.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: qtreemaparea.cpp,v 1.7 2001/07/11 19:54:49 alexannika Exp $
 *
 */

#include <string.h>
#include <sys/errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <qtimer.h>
#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>
//#include "kdirtree.h"
//#include "kdirtreeview.h"
//#include "kdirsaver.h"
#include "qtreemap.h"
#include <qmainwindow.h>
//#include <bits/mathcalls.h>

using namespace KDirStat;

QTreeMapArea::QTreeMapArea(QWidget *parent) : QWidget(parent) {
  options=new QTreeMapOptions();

  offscreen=QPixmap(options->paint_size_x,options->paint_size_y);
  //  offscreenptr=new  QPixmap(options->paint_size_x,options->paint_size_y);
  // offscreen=*offscreenptr;

  //offscreenptr=NULL;

  //painter=new QPainter(&offscreen);
  //win_painter=new QPainter(this);
  painter=new QPainter();
  win_painter=new QPainter();

  setMouseTracking(TRUE);
  root_tree=NULL;

  default_color=QColor(255,255,255);

  color_index=0;
  
  rotate_colors[0]=QColor(255,0,0);
  rotate_colors[1]=QColor(255,255,0);
  rotate_colors[2]=QColor(255,0,255);
  rotate_colors[3]=QColor(0,255,0);
  rotate_colors[4]=QColor(0,0,255);
#define LAST_ROTATING_COL 4

  tooltipBgColor=QColor(200,200,0);
  tooltipFgColor=QColor(0,0,0);

  dirBaseColor=QColor(255,255,255);

  this->resize(options->paint_size_x,options->paint_size_y);
  this->setBackgroundMode(PaletteBackground);
}

int QTreeMapArea::getNextRotatingColorIndex(){
  color_index++;
  if(color_index>LAST_ROTATING_COL){
    color_index=0;
  }
  return color_index;
}

void QTreeMapArea::setTreeMap(Object *dutree){
  root_tree=dutree;

  if(root_tree){
    drawTreeMap(dutree);

    emit changedDirectory(dutree);
  }
}

QColor&  QTreeMapArea::getBaseColor(QString name){
  QColor basecolor;

  if(options->color_scheme==CS_CYCLIC){
	basecolor=QColor(rotate_colors[color_index]);
	getNextRotatingColorIndex();
  }
  else if(options->color_scheme==CS_MONO){
    basecolor=options->mono_color;
  }
  else if(options->color_scheme==CS_REGEXP){
    QRegExp r1("\\.html");
    QRegExp r2("\\.exe");
    QRegExp r3("\\.so");
    QRegExp r4("\\.o");

    if(r1.match(name)!=-1){
      basecolor=QColor(255,0,0);
      printf("match %s\n",name.latin1());
    }
    else if(r1.match(name)!=-1){
      basecolor=QColor(0,255,0);
    }
    else if(r1.match(name)!=-1){
      basecolor=QColor(255,255,0);
    }
    else if(r1.match(name)!=-1){
      basecolor=QColor(0,255,255);
    }
    else{
      basecolor=QColor(255,255,255);
    }
  }

  return basecolor;
}

Object *QTreeMapArea::findClickedMap(Object *dutree,int x,int y,int findmode){
  //Object *found;

  found_kfileinfo=NULL;

  //  painter->end();

  // copy offscreen to widget
  //win_painter->end();
  win_painter->begin(this);
  win_painter->drawPixmap(0,0,offscreen,0,0,options->paint_size_x,options->paint_size_y);
  win_painter->flush();
  win_painter->end();

    // draw outlines on widget, not on offscreen
    painter->begin(this);
    drawDuTree(dutree,0,0,options->paint_size_x,options->paint_size_y,options->start_direction,0,NULL,x,y,findmode);

  //printf("FOUND: %s\n",found_kfileinfo->debugUrl().latin1());

    int tf=0;

    if(y>(options->paint_size_y - 50)){
      y=y-20;
    }
       else{
	 y=y+20;
       }
    if(x>(options->paint_size_x/2)){
      x=x-20;
      tf=AlignRight;
    }
       else{
	 x=x+20;
       }

    int size=totalSize(found_kfileinfo);
    //QString text=QString(found_kfileinfo->debugUrl()+" size: "+tellUnit(size));
    
    QString text; //=found_kfileinfo->debugUrl();
    //text.sprintf("%s %d bytes",found_kfileinfo->debugUrl().latin1(),size);
    text.sprintf("%s %s",fullName(found_kfileinfo).latin1(),
		 tellUnit(size).latin1());

    QRect tt_rect=painter->boundingRect(x,y,-1,-1,tf,text);

    painter->fillRect(tt_rect,tooltipBgColor);
    mypen.setColor(tooltipFgColor);
    painter->setPen (mypen);
    painter->drawText(tt_rect,tf,text);

    painter->end();

  return found_kfileinfo;
}

void QTreeMapArea::paintEvent( QPaintEvent *event){
  NOT_USED(event);

  if(root_tree!=NULL){
    //    QRect event_rect=event->rect();

    //    painter->end(); // end painting on offscreen

    // copy offscreen to widget
    //win_painter->end();
    win_painter->begin(this);
    win_painter->drawPixmap(0,0,offscreen,0,0,options->paint_size_x,options->paint_size_y);
    win_painter->flush();
    win_painter->end();

    //painter->begin(this);
  }
}

void QTreeMapArea::mousePressEvent(QMouseEvent *mouse){
  //   kdDebug() << k_funcinfo << endl;
  if(root_tree!=NULL && mouse->button()==RightButton){
    int x=mouse->x();
    int y=mouse->y();
    if( (0<=x && x<=options->paint_size_x) && (0<=y && y<=options->paint_size_y)){
      // find the first dir (the first coord. matching entry) the mouse points to
      Object *found=findClickedMap(root_tree,x,y,FIND_FILE);
      
      if(found!=NULL){
    kdDebug() << k_funcinfo << endl;
    pop=new QPopupMenu(this); // memory hole!
	pop->insertTearOffHandle();
	pop->insertItem(fullName(found));
	pop->insertSeparator();
	
	Object *walk=found;
	while(walk!=NULL){
	  QPopupMenu *popwalk=new QPopupMenu(this); // memory hole!
	  //popwalk->insertTearOffHandle();

	  int delete_id=popwalk->insertItem("delete",this, SLOT(deleteFile(int))); // howto?
	  popwalk->setItemParameter(delete_id,(int)walk);  // ouch!

	  int xterm_id=popwalk->insertItem("xterm",this, SLOT(shellWindow(int))); // howto?
	  popwalk->setItemParameter(xterm_id,(int)walk);  // ouch!

	  int konq_id=popwalk->insertItem("konqueror",this, SLOT(browserWindow(int))); // howto?
	  popwalk->setItemParameter(konq_id,(int)walk);  // ouch!

	  QString text;
	  text.sprintf("%-20s %5s",fullName(walk).latin1(),tellUnit(totalSize(walk)).latin1());
	  pop->insertItem(text,popwalk);

	  walk=parentNode(walk);
	}
	pop->popup(this->mapToGlobal(QPoint(x,y)));
      }
    }
  }
}

void QTreeMapArea::mouseDoubleClickEvent(QMouseEvent *mouse){
  if(root_tree!=NULL){
    int x=mouse->x();
    int y=mouse->y();
    if( (0<=x && x<=options->paint_size_x) && (0<=y && y<=options->paint_size_y)){
      // find the first dir (the first coord. matching entry) the mouse points to
      Object *found=findClickedMap(root_tree,x,y,FIND_FIRSTDIR);
      
      if(found!=NULL){
	// draw the new map
	setTreeMap(found);
      }
    }
  }
}

void QTreeMapArea::mouseMoveEvent(QMouseEvent *mouse){
  if(root_tree!=NULL){
    int x=mouse->x();
    int y=mouse->y();
    if( (0<=x && x<=options->paint_size_x) && (0<=y && y<=options->paint_size_y)){
      // find the file (the last coord. matching entry) the mouse points to
      Object *found=findClickedMap(root_tree,x,y,FIND_FILE);
      emit highlighted(found);
    }
  }
}

void QTreeMapArea::keyPressEvent(QKeyEvent *event){
  QString key=event->text();
  if(key==QString("u")){
    directoryUp();
  }
}

void QTreeMapArea::resizeEvent( QResizeEvent *ev)
{
    // kdDebug() << k_funcinfo << endl;
  NOT_USED(ev);

    update();   // Trigger repaint on resize
}

void QTreeMapArea::buttonUp(){
  directoryUp();
}

void QTreeMapArea::directoryUp(){
  Object *parent=parentNode(root_tree);
  if(parent!=NULL){
    setTreeMap(parent);
  }
}

void QTreeMapArea::setOptions(QTreeMapOptions *opt){
  delete options;

  options=new QTreeMapOptions(*opt);
  if(root_tree){
    drawTreeMap(root_tree);
  }
}

QTreeMapOptions *QTreeMapArea::getOptions(){
  return options;
}


void QTreeMapArea::zoomIn(){
  options->paint_size_x*=2;
  options->paint_size_y*=2;

  this->resize(options->paint_size_x,options->paint_size_y);
  //painter->end();
  //painter->begin(&offscreen);
  drawTreeMap(root_tree);
  this->repaint();
}
void QTreeMapArea::zoomOut(){
  options->paint_size_x/=2;
  options->paint_size_y/=2;

  this->resize(options->paint_size_x,options->paint_size_y);
  //painter->end();
  //painter->begin(&offscreen);
  drawTreeMap(root_tree);
  this->repaint();
}


void QTreeMapArea::deleteFile(){
}

void QTreeMapArea::deleteFile(int id){
  deleteFile((Object *)id);
}

void QTreeMapArea::deleteFile(Object *convicted){
  printf("delete File %s\n",fullName(convicted).latin1());
}

void QTreeMapArea::shellWindow(int id){
  Object *dir=(Object *)id;

  if(isNode(dir)){
    //system("xterm "+dir->url()+" &"); // this should of course be changed!
    system("( cd "+fullName(dir)+" ; xterm ) &"); // this should of course be changed!
  }
}
void QTreeMapArea::browserWindow(int id){
  Object *dir=(Object *)id;

    system("konqueror "+fullName(dir)+" &"); // this should of course be changed!
}

int QTreeMapArea::check_int(int i){
  if(i<0){
    i=0;
  }
  if(i>=256){
    i=255;
  }
  if(isnan(i)){
    i=0;
  }
  return i;
}

QTreeMapArea::~QTreeMapArea()
{
  // to be filled later...
}


QTreeMapOptions::QTreeMapOptions(){
  draw_mode=DM_BOTH;
  paintmode=PM_FLAT;
  start_direction=HORIZONTAL;
  step_width=0;
  paint_size_x=800;
  paint_size_y=600;
  highlight_frame_width=3;
  highlight_frame_col=QColor(255,255,255);
  dont_draw_xyd=0;
  dont_descend_xyd=0;
  draw_text=FALSE;
  color_scheme=CS_CYCLIC;
  hc_factor=0.3f;
  border_step=0;
  dynamic_shading=FALSE;
  mono_color=QColor(255,255,255);
  sequoia_f=0.75;
  sequoia_h=0.5;
  squarify=FALSE;
  show_inodes=TRUE;
}

