/*
 *   File name:	qtreemaparea.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: qtreemaparea.cpp,v 1.1 2001/06/29 16:37:50 hundhammer Exp $
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

void QTreeMapArea::setTreeMap(KDirInfo *dutree){
  root_tree=dutree;

  drawTreeMap(dutree);

  emit changedDirectory(dutree);
}

void QTreeMapArea::drawTreeMap(KDirInfo *dutree){
  int x0=0;
  int y0=0;
  int xd0=options->paint_size_x;
  int yd0=options->paint_size_y;

  if(offscreenptr!=NULL){
    delete offscreenptr;
  }
  //offscreenptr=new  QPixmap(options->paint_size_x,options->paint_size_y);
  //offscreen=*offscreenptr;

  offscreen.resize(options->paint_size_x,options->paint_size_y);
  color_index=0;

  painter->begin(&offscreen);
  //painter->setWindow(x0,y0,xd0,yd0);
  painter->setFont(QFont( "times",10));

  painter->eraseRect(0,0,options->paint_size_x,options->paint_size_y);

  Cushion *cushion=NULL;
  if(options->paintmode==PM_CUSHION){
    cushion=new Cushion(xd0,yd0);
  }
  else{
    cushion=NULL;
  }
    drawDuTree(dutree,x0,y0,xd0,yd0,options->start_direction,0,cushion);

    painter->end();

    this->update();
    this->repaint();
    //    graph_widget->repaint();
}

//void QTreeMapArea::drawDuTree(KDirInfo *dutree, int x0,int y0,int xd0, int yd0, bool direction, int level,Cushion *cushion,int fx=-1,int fy=-1,int findmode=FIND_NOTHING){
void QTreeMapArea::drawDuTree(KDirInfo *dutree, int x0,int y0,int xd0, int yd0, bool direction, int level,Cushion *cushion,int fx,int fy,int findmode){
  QString node_name=dutree->debugUrl();
  KDirInfo *sub_nodes=(KDirInfo *)dutree->firstChild();
  int node_totalsize=dutree->totalSize();
  Cushion *c=NULL;

  //    printf("QTreeMapArea::drawDuTree(%s,%d,%d,%d,%d,dir=%d,level=%d) %d\n",node_name.latin1(),x0,y0,xd0,yd0,direction,level,node_totalsize);

  if(xd0>=options->dont_draw_xyd && yd0>=options->dont_draw_xyd){
  if(fx>=0 && fy>=0){
    // search mode
    if( ( x0<=fx && fx<=(x0+xd0) ) && ( ( y0<=fy && fy<=(y0+yd0)) ) ){
      // mouse coord are inside this entries coordinates
      found_kfileinfo=dutree;
      if(findmode==FIND_FIRSTDIR && dutree!=root_tree){
	// we have found the first directory with this coordinates
	return;
      }
    }
    else{
      // don't descend into this part of tree
      return;
    }
  }
  else{
    // not in search mode
    if(cushion!=NULL){
      c=new Cushion(*cushion);
      if(dutree!=root_tree){
	cushion_AddRidge(c->r[direction][0],
			 c->r[direction][1],
			 c->h,
			 c->s[direction][1],
			 c->s[direction][2]);
      }
    }
    else{
      c=NULL;
    }
  }

  if((options->draw_mode==DM_FILES || options->draw_mode==DM_BOTH )&& dutree->isFile() /* && (fx==-1 && fy==-1) */ ){
	QColor basecolor;
	//basecolor=getBaseColor(node_name);
	basecolor=QColor(rotate_colors[color_index]);
	getNextRotatingColorIndex();

	int pmode;
	if((fx>=0 && fy>=0)){
	  // when searching, only draw outline
	    pmode=PM_OUTLINE;
	    if(dutree==found_kfileinfo){
	      paintEntry(x0,y0,xd0,yd0,node_name,direction,level,options->highlight_frame_col,pmode,c);
	  }
	}
	else{
	  // really draw this entry
	  pmode=options->paintmode;
	  paintEntry(x0,y0,xd0,yd0,node_name,direction,level,basecolor,pmode,c);
	}

      }
      else if((options->draw_mode==DM_DIRS || options->draw_mode==DM_BOTH) && ( dutree->isDir() || dutree->isDotEntry())) {
#define MDEPTH 6
	//QColor basecolor=QColor(rotate_colors[color_index]);
	QColor basecolor=QColor(dirBaseColor);
	float i=((float)(MDEPTH-level))/((float)MDEPTH);
	QColor dircolor=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
	//getNextRotatingColorIndex();

	//KDirInfo *dirinfo=(KDirInfo *)dutree;

	int pmode;
	if(fx>=0 && fy>=0){
	  // draw outline in search mode
	  pmode=PM_OUTLINE;
	  if(dutree==found_kfileinfo){
	    paintEntry(x0,y0,xd0,yd0,node_name,direction,level,options->highlight_frame_col,pmode,c);
	  }
	}
	else{
	  // draw the rectangle
	  //pmode=options->paintmode;
	  pmode=PM_FLAT; // directories are never drawn shaded
	  paintEntry(x0,y0,xd0,yd0,node_name,direction,level,dircolor,pmode,c);
	}
      }
      else{
	// we've found a symbolic link or like that
	//printf("neither file nor dir %s\n",node_name.latin1());
	// do nothing?
      }
      
      float x=(float)x0;
      float y=(float)y0;
      float xd=0.0;
      float yd=0.0;
      float w=0.0;

      if(c!=NULL){
	// cushion mode
	w=( c->r[direction][1] - c->r[direction][0] )/((float)node_totalsize);
      }
      for(KDirInfo *subtree=sub_nodes;subtree!=NULL;subtree=(KDirInfo *)subtree->next()){
	int subnode_size=subtree->totalSize();
	  if(subnode_size==0){
	    // we do not descend in directories with 0 size
	  }
	  else{
	    float percent_size=((float)subnode_size)/((float)node_totalsize);
	    
	    if(direction==HORIZONTAL){
	      // horizontal
	      xd=(float)xd0;
	      yd=(float)(((float)yd0)*percent_size);
	    }
	    else{
	      // vertikal
	      xd=(float)(((float)xd0)*percent_size);
	      yd=(float)yd0;
	    }
	    bool subdirection=!direction;
	    
	    int sw=options->step_width;

	    if(c!=NULL){
	      c->r[direction][1]=c->r[direction][0] + ( w*((float)subnode_size) );
	      c->h=c->h*c->f;
	    }
	    
	    drawDuTree(subtree,(int)(x+sw),(int)(y+sw),(int)(xd-sw),(int)(yd-sw),subdirection,level+1,c,fx,fy,findmode);

	    if(c!=NULL){
	      c->r[direction][0]=c->r[direction][1];
	    }

	    if(direction==HORIZONTAL){
	      // horizontal
	      y=y+yd;
	    }
	    else{
	      x=x+xd;
	    }
	  }
      }

      if( dutree->dotEntry()!=NULL ){
	KDirInfo *dotentry=(KDirInfo *)dutree->dotEntry();
	int subnode_size=dotentry->totalSize();
	    float percent_size=((float)subnode_size)/((float)node_totalsize);
	    
	    if(direction==HORIZONTAL){
	      // horizontal
	      xd=(float)xd0;
	      yd=(float)(((float)yd0)*percent_size);
	    }
	    else{
	      // vertikal
	      xd=(float)(((float)xd0)*percent_size);
	      yd=(float)yd0;
	    }
	    bool subdirection=direction;
	    
	    int sw=options->step_width;

	    drawDuTree(dotentry,(int)x,(int)y,(int)(xd-sw),(int)(yd-sw),subdirection,level,c,fx,fy,findmode);
      }
      if(c!=NULL){
	delete c;
      }
  }
}

void QTreeMapArea::paintEntry(int x0, int y0, int xd, int yd,QString entry_name,bool direction,int level,const QColor &basecolor,int pmode,Cushion *c){

  NOT_USED(direction);
  NOT_USED(level);

#define MPIX 1

  if(xd<=MPIX || yd<=MPIX){
    if(xd<=MPIX){
      xd=MPIX;
    }
    if(yd<=MPIX){
      yd=MPIX;
    }
  }

#if 0
      mypen.setColor(QColor(0,255,0));
      mypen.setWidth( 4);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );

    painter->drawLine(x0,y0,x0+xd,y0+yd);
#endif

  if(pmode==PM_OUTLINE){
    // draw highlighted frames in search mode
    mypen.setColor(basecolor);
    mypen.setWidth(options->highlight_frame_width);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );

    painter->drawRect(x0,y0,xd,yd);
    //painter->flush();
  }
  else if(pmode==PM_FLAT){
    painter->fillRect(x0,y0,xd,yd,basecolor);
    //painter->flush();
  }
  else if(pmode==PM_PYRAMID){
    int step=0;
    int maxstep;

    if(xd>yd){
      maxstep=yd/2;
    }
    else{
      maxstep=xd/2;
    }

    while(step<maxstep){
      QColor color=QColor((basecolor.red()/maxstep)*(maxstep-step),
			  (basecolor.green()/maxstep)*(maxstep-step),
			  (basecolor.blue()/maxstep)*(maxstep-step));
      mypen.setColor(color);
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );
      // why does this drawRect not work?
      // it draws only the left and upper side line
      // but not the bottom and right side line
      painter->drawRect(x0+step,y0+step,xd-step,yd-step);
      //painter->flush();
      step++;
    }
  }
  else if(pmode==PM_SIMPLE_CUSHION){
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float ix=sin(((float)w)/((float)xd)*3.14f);
	float iy=sin(((float)h)/((float)yd)*3.14f);
	float i=(ix+iy)/2.0; // intensity as a sin in x/y direction
	QColor newcol=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
      mypen.setColor(newcol);
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );
	
      painter->drawPoint(x0+w,y0+h);
      //painter->flush();
      }
    }
  }
  else if(pmode==PM_SQUARE_CUSHION){
#define scfunc(x)  (-(x*x)+1.0)

    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float div_x=((float)w)/((float)xd); // range 0...1
	float div_y=((float)h)/((float)yd); // range 0...1

	float step1_x=(div_x-0.5)*2.0; // range -1...1
	float step1_y=(div_y-0.5)*2.0; // range -1...1

	float res_x=scfunc(step1_x); // range 0...1...0
	float res_y=scfunc(step1_y);

	float ix=res_x;
	float iy=res_y;
	float i=(ix+iy)/2.0;
	QColor newcol=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
      mypen.setColor(newcol);
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );
	
      painter->drawPoint(x0+w,y0+h);
      //painter->flush();
      }
    }
  }
  else if(pmode==PM_CONE_CUSHION){
#define ccfunc(x)  (-fabs(x)+1)

    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float div_x=((float)w)/((float)xd); // range 0...1
	float div_y=((float)h)/((float)yd); // range 0...1

	float step1_x=(div_x-0.5)*2.0; // range -1...1
	float step1_y=(div_y-0.5)*2.0; // range -1...1

	float res_x=ccfunc(step1_x); // range 0...1...0
	float res_y=ccfunc(step1_y);

	float ix=res_x;
	float iy=res_y;
	float i=(ix+iy)/2.0;
	QColor newcol=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
      mypen.setColor(newcol);
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );
	
      painter->drawPoint(x0+w,y0+h);
      //painter->flush();
      }
    }
  }
  else if(pmode==PM_WAVE_CUSHION){
    //#define wcfunc(x)  (sin(((x+1.0)/2.0)*3.14))
    //#define wcfunc(x)  (((sin(((x+1.0)/2.0)*3.14))/2.0)+0.3)
#define wcfunc(x)  (((sin(((x+1.0)/2.0)*3.14))/2.0)+0.3)

    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float div_x=((float)w)/((float)xd); // range 0...1
	float div_y=((float)h)/((float)yd); // range 0...1

	float step1_x=(div_x-0.5)*2.0; // range -1...1
	float step1_y=(div_y-0.5)*2.0; // range -1...1

	float res_x=wcfunc(step1_x); // range 0...1...0
	float res_y=wcfunc(step1_y);

	float ix=res_x;
	float iy=res_y;
	float i=(ix+iy)/2.0;
	QColor newcol=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
      mypen.setColor(newcol);
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );
	
      painter->drawPoint(x0+w,y0+h);
      //painter->flush();
      }
    }
  }
  else if(pmode==PM_BITMAP){
  }
  else if(pmode==PM_CUSHION && c!=NULL){
    float Ia=40.0;
    float Is=215.0;
    float Lx=0.09759;
    float Ly=0.19518;
    float Lz=0.9759;

    //for(float iy=c->r[DY][0] + 0.5 ; iy<=( c->r[DY][1] - 0.5 );iy++){
    //  for(float ix=c->r[DX][0] + 0.5 ; ix<=( c->r[DX][1] - 0.5 );ix++){
    for(float iy=y0;iy<=y0+yd;iy++){
      for(float ix=x0;ix<=x0+xd;ix++){
	float nx=-(2*c->s[DX][2]*(ix+0.5) + c->s[DX][1] );
	float ny=-(2*c->s[DY][2]*(iy+0.5) + c->s[DY][1] );
	float cosa=(nx*Lx + ny*Ly + Lz )/sqrt(nx*nx + ny*ny +1.0);

	float i=Ia+MAX(0,Is*cosa); // range?
	i=i/512.0;
	i=MIN(1.0,i);
	i=MAX(0.0,i);

	//printf("cushion: i=%f\n",i);
	QColor newcol=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
      mypen.setColor(newcol);
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );

	painter->drawPoint((int)ix,(int)iy);
      }
    }
      
  }
  else{
    printf("QTreeMapArea::paintEntry no option\n");
  }

  if(pmode!=PM_OUTLINE && options->draw_text){
    // draw text inside rectangle if text fits
      mypen.setColor(QColor(255,255,255));
      mypen.setWidth( 1);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );

      //if(direction==VERTIKAL && yd>5){
    if(xd>=yd && yd>5){
      int maxlen=xd/4;
      QString end=entry_name.right(maxlen);
      painter->drawText(x0,y0+8,end,maxlen);
    }
    //else if(direction==HORIZONTAL && xd>5){
    else if(xd<yd && xd>5){
      int maxlen=yd/4;

      // draw text rotated
      painter->save();
      painter->translate(x0,y0);
      painter->rotate(90);


      QString end=entry_name.right(maxlen);
      painter->drawText( 0,0,end,maxlen);
      painter->restore();
    }
  }
}

#if 0
QColor&  getBaseColor(QString name){
  QColor basecolor;
	/*
	if(isExecutable()){
	  basecolor=NULL;
	}
	else{
	  basecolor=getBaseColorByName(node_name);
	}
	*/
  basecolor=QColor( rand()&255 , rand()&255, rand()&255 );

  return basecolor;
}
#endif

KDirInfo *QTreeMapArea::findClickedMap(KDirInfo *dutree,int x,int y,int findmode){
  //KDirInfo *found;

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

    int size=found_kfileinfo->totalSize();
    //QString text=QString(found_kfileinfo->debugUrl()+" size: "+tellUnit(size));
    
    QString text; //=found_kfileinfo->debugUrl();
    //text.sprintf("%s %d bytes",found_kfileinfo->debugUrl().latin1(),size);
    text.sprintf("%s %s",found_kfileinfo->debugUrl().latin1(),
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
      KDirInfo *found=findClickedMap(root_tree,x,y,FIND_FILE);
      
      if(found!=NULL){
    kdDebug() << k_funcinfo << endl;
    pop=new QPopupMenu(this); // memory hole!
	pop->insertTearOffHandle();
	pop->insertItem(found->debugUrl());
	pop->insertSeparator();
	
	KDirInfo *walk=found;
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
	  text.sprintf("%-20s %5s",walk->debugUrl().latin1(),tellUnit(walk->totalSize()).latin1());
	  pop->insertItem(text,popwalk);

	  walk=(KDirInfo *)walk->parent();
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
      KDirInfo *found=findClickedMap(root_tree,x,y,FIND_FIRSTDIR);
      
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
      KDirInfo *found=findClickedMap(root_tree,x,y,FIND_FILE);
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
  KDirInfo *parent=(KDirInfo *)root_tree->parent();
  if(parent!=NULL){
    setTreeMap(parent);
  }
}

void QTreeMapArea::cushion_AddRidge(float x1,float x2,float h,float& s1,float& s2){
  s1=s1+4*h*(x2+x1)/(x2-x1);
  s2=s2-4*h/(x2-x1);
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

QString QTreeMapArea::tellUnit(int size){
  QString str;
  if(size<1024){
    str.sprintf("%d bytes",size);
  }
  else if(size<(1024*1024)){
    str.sprintf("%d kB",size/(1024));
  }
  else if(size<(1024*1024*1024)){
    str.sprintf("%d MB",size/(1024*1024));
  }
  else {
    str.sprintf("%d GB",size/(1024*1024*1024));
  }

  return str;
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
  deleteFile((KDirInfo *)id);
}

void QTreeMapArea::deleteFile(KDirInfo *convicted){
  printf("delete File %s\n",convicted->debugUrl().latin1());
}

void QTreeMapArea::shellWindow(int id){
  KDirInfo *dir=(KDirInfo *)id;

  if(dir->isDir()){
    //system("xterm "+dir->url()+" &"); // this should of course be changed!
    system("( cd "+dir->url()+" ; xterm ) &"); // this should of course be changed!
  }
}
void QTreeMapArea::browserWindow(int id){
  KDirInfo *dir=(KDirInfo *)id;

    system("konqueror "+dir->url()+" &"); // this should of course be changed!
}

QTreeMapArea::~QTreeMapArea()
{
  // to be filled later...
}

Cushion::Cushion(int xd,int yd){
  r[DX][0]=0.0;
  r[DX][1]=(float)xd;
  r[DY][0]=0.0;
  r[DY][1]=(float)yd;

  s[DX][1]=0.0;
  s[DX][2]=0.0;
  s[DY][1]=0.0;
  s[DY][2]=0.0;

  h=0.5;
  f=0.75;
}


QTreeMapOptions::QTreeMapOptions(){
  draw_mode=DM_BOTH;
  paintmode=PM_FLAT;
  start_direction=HORIZONTAL;
  step_width=0;
  paint_size_x=800;
  paint_size_y=500;
  highlight_frame_width=3;
  highlight_frame_col=QColor(255,255,255);
  dont_draw_xyd=0;
  dont_descend_xyd=1;
  draw_text=TRUE;
  color_scheme=CS_CYCLIC;
}
