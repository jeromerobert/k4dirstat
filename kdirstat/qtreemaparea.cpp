/*
 *   File name:	qtreemaparea.cpp
 *   Summary:	
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: qtreemaparea.cpp,v 1.16 2001/08/10 03:45:48 alexannika Exp $
 *
 */

#include <string.h>
#include <sys/errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <qtimer.h>
#include "qtreemap.h"
#include <qmainwindow.h>
#include <qfiledialog.h>
#include <qfile.h>
#include "qtreemapwindow.h"
#include "qxmltreemap.h"
#include "qxmltreemapwindow.h"
#include <iostream.h>

//using namespace KDirStat;

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
  flag_middle_button=FALSE;
  offset_x=0;
  offset_y=0;

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

  selected_list=new ObjList(this);
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

    dirChange(dutree);

    emit changedDirectory(dutree);
  }
}

QColor  QTreeMapArea::getBaseColor(QString name){
  QColor bcolor;

  if(options->color_scheme==CS_CYCLIC){
	bcolor=QColor(rotate_colors[color_index]);
	getNextRotatingColorIndex();
  }
  else if(options->color_scheme==CS_MONO){
    bcolor=options->mono_color;
  }
  else if(options->color_scheme==CS_REGEXP){

    if(options->scheme_list!=NULL){
      bool found=FALSE;
            bcolor=options->mono_color;


      for(uint i=0;i<options->scheme_list->count() && found==FALSE;i++){
	QTMcolorScheme *scheme=options->scheme_list->at(i);
	QList<QRegExp> *rlist=scheme->regexplist;

	for(uint j=0;j<rlist->count() && found==FALSE;j++){
	  if(rlist->at(j)->match(name)!=-1){
	    //	    printf("found pattern %s on name %s %d %d %d\n",rlist->at(j)->pattern().latin1(),name.latin1(),scheme->color.red(),scheme->color.green(),scheme->color.blue());
	    //	    printf("found on name %s %d %d %d\n",name.latin1(),scheme->color.red(),scheme->color.green(),scheme->color.blue());
	    bcolor=QColor(scheme->color);
	    found=TRUE;
	  }
	}
      }
    }
  }


  return bcolor;
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

    find_x=x; // global values
    find_y=y;
    find_mode=findmode;

    if(options->piemap==TRUE){
#ifdef HAVE_PIEMAP
      found_kfileinfo=root_tree;
      Cushion *cushion=new Cushion(options->paint_size_x,options->paint_size_y,options->sequoia_h,options->sequoia_f);
      drawPieMap(dutree,cushion);
      delete cushion;
#endif
    }
    else{
      drawDuTree(dutree,0,0,options->paint_size_x,options->paint_size_y,options->start_direction,0,NULL,x,y,findmode);
    }
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

    asize size=totalSize(found_kfileinfo);
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

void QTreeMapArea::toggleSelection(Object *found){
      if(found!=NULL){
	if(selected_list->containsRef((Object **)found)){
	  selected_list->removeRef((Object **)found);
	  //	  printf("removed %s\n",fullName(found).latin1());
	}
	else{
	  selected_list->append((Object **)found);
	  //printf("appended %s\n",fullName(found).latin1());
	}
      }
}


void QTreeMapArea::mousePressEvent(QMouseEvent *mouse){
  //   kdDebug() << k_funcinfo << endl;
    int x=mouse->x();
    int y=mouse->y();
    if(root_tree!=NULL && mouse->button()==MidButton){
      //printf("pressed mid\n");
      flag_middle_button=TRUE;
      middle_x=x;
      middle_y=y;
    }
  else if(root_tree!=NULL && mouse->button()==LeftButton){
    if( (0<=x && x<=options->paint_size_x) && (0<=y && y<=options->paint_size_y)){
      // find the first dir (the first coord. matching entry) the mouse points to
      Object *found=findClickedMap(root_tree,x,y,FIND_FILE);
     
      toggleSelection(found);

  Object *dutree=root_tree;

    painter->begin(&offscreen);

    Cushion *cushion=new Cushion(options->paint_size_x,options->paint_size_y,options->sequoia_h,options->sequoia_f);

      drawDuTree(dutree,0,0,options->paint_size_x,options->paint_size_y,options->start_direction,0,cushion,x,y,FIND_SELECTION);

    delete cushion;

    painter->end();

  win_painter->begin(this);
  win_painter->drawPixmap(0,0,offscreen,0,0,options->paint_size_x,options->paint_size_y);
  win_painter->flush();
  win_painter->end();

    }

  }
  else if(root_tree!=NULL && mouse->button()==RightButton){
    if( (0<=x && x<=options->paint_size_x) && (0<=y && y<=options->paint_size_y)){
      // find the first dir (the first coord. matching entry) the mouse points to
      Object *found=findClickedMap(root_tree,x,y,FIND_FILE);
      
      if(found!=NULL){
	//    kdDebug() << k_funcinfo << endl;
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
	
	QPopupMenu *popselection=new QPopupMenu(this); // memory hole!
	for(uint i=0;i<selected_list->count();i++){
	  Object *walk=(Object *)selected_list->at(i);


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
	  /* int id= */ popselection->insertItem(text,popwalk);
	}
	pop->insertItem("selection:",popselection);
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
      Object *found_dir=findClickedMap(root_tree,x,y,FIND_FIRSTDIR);

      Object *found_file=findClickedMap(root_tree,x,y,FIND_FILE);

      toggleSelection(found_file);

      if(found_dir!=NULL){
	// draw the new map
	setTreeMap(found_dir);
      }
    }
  }
}

void QTreeMapArea::mouseMoveEvent(QMouseEvent *mouse){
  if(root_tree!=NULL){
    int x=mouse->x();
    int y=mouse->y();
    if( (0<=x && x<=options->paint_size_x) && (0<=y && y<=options->paint_size_y)){
      //      if(flag_middle_button==TRUE){
      if(mouse->state()==MidButton && options->piemap==TRUE){
#ifdef HAVE_PIEMAP
	int dx=-(middle_x-x);
	int dy=-(middle_y-y);
	offset_x+=dx;
	offset_y+=dy;
       
	//	printf("dragging m.button offset=%d:%d d=%d:%d\n",offset_x,offset_y,dx,dy);
	middle_x=x;
	middle_y=y;
    painter->begin(&offscreen);
    painter->eraseRect(0,0,options->paint_size_x,options->paint_size_y);

	Cushion *cushion=new Cushion(options->paint_size_x,options->paint_size_y,options->sequoia_h,options->sequoia_f);
	find_mode=-1;
	drawPieMap(root_tree,cushion);

    painter->end();

  win_painter->begin(this);
  win_painter->drawPixmap(0,0,offscreen,0,0,options->paint_size_x,options->paint_size_y);
  win_painter->flush();
  win_painter->end();
#endif
      }
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

void QTreeMapArea::findMatch(const QString find){
  found_kfileinfo=NULL;
  find_regexp=QRegExp(find,FALSE,FALSE);

  Object *dutree=root_tree;

    painter->begin(&offscreen);

    Cushion *cushion=new Cushion(options->paint_size_x,options->paint_size_y,options->sequoia_h,options->sequoia_f);

    if(find==""){
      // redraw clean
      drawDuTree(dutree,0,0,options->paint_size_x,options->paint_size_y,options->start_direction,0,cushion);
    }
    else{
      drawDuTree(dutree,0,0,options->paint_size_x,options->paint_size_y,options->start_direction,0,cushion,-1,-1,FIND_MATCH);
    }

    delete cushion;

    painter->end();

  win_painter->begin(this);
  win_painter->drawPixmap(0,0,offscreen,0,0,options->paint_size_x,options->paint_size_y);
  win_painter->flush();
  win_painter->end();

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
  printf("delete File %s - not yet implemented\n",fullName(convicted).latin1());
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

QString QTreeMapArea::findFullName(Object *node){
  Object *parent=parentNode(node);
  QString fullname;
  if(parent!=NULL){
    fullname=findFullName(parent)+options->path_separator+shortName(node);
  }
  else{
    fullname=shortName(node);
  }
  return fullname;
}

asize QTreeMapArea::calcTotalSize(Object *node){
  if(isLeaf(node)){
    return totalSize(node);
  }

    Object *child=firstChild(node);
    bool dotentry_flag=FALSE;
    asize size=0.0;

    while(child!=NULL){
      size+=calcTotalSize(child);
      child=nextChild(child);
	      if(child==NULL && dotentry_flag==FALSE){
		dotentry_flag=TRUE;
		Object *dotentry=sameLevelChild(node);
		if(dotentry){
		  child=firstChild(dotentry);
		}
	      }
     }

    return size;
}

asize QTreeMapArea::areaSize(Object *node){
  asize size=0.0;

  if(options->area_is==AREA_IS_TOTALSIZE){
    size=totalSize(node);
  }
  else if(options->area_is==AREA_IS_TOTALITEMS){
    size=(asize)totalItems(node);
  }
  else{
    //error
    exit(0);
  }
  return size;
}

void QTreeMapArea::saveAsBitmap(){
  QString filename=QFileDialog::getSaveFileName("treemap.png", "Images (*.png *.xpm *.jpg)" , this);

  if(!filename.isEmpty()){
    //offscreen.save(filename,QImageIO::imageFormat(filename));
    offscreen.save(filename,"PNG");
  }
}
void QTreeMapArea::saveAsXML(QTextStream& file,Object *tree,int level){
  //  QTextStream file=*fileptr;
  //  file << "<node

  QString ident=QString();
  ident.fill(' ',level);

  file << ident << "<node name=\"" << shortName(tree) << "\" size=\"" << \
	totalSize(tree) << "\"" ;

  if(isLeaf(tree)){
    file << "/>" << endl;// << flush;
  }
     else{
       file << ">" << endl;// << flush;

    Object *child=firstChild(tree);
    bool dotentry_flag=FALSE;
    while(child!=NULL){
      //    printf("XML: %s\n",shortName(child).latin1());

      saveAsXML(file,child,level+1);
	      child=nextChild(child);

	      if(child==NULL && dotentry_flag==FALSE){
		dotentry_flag=TRUE;
		Object *dotentry=sameLevelChild(tree);
		if(dotentry){
		  child=firstChild(dotentry);
		}
	      }
    }
    file << ident << "</node>" << endl;
     }

}

void QTreeMapArea::saveAsXML(){
  QString filename=QFileDialog::getSaveFileName("treemap.xml", "XML (*.xml)" , this);

  if(!filename.isEmpty()){
  QFile *f=new QFile(filename);
  f->open(IO_WriteOnly);

  QTextStream file(f);//,IO_WriteOnly);

  //  printf("filename: %s\n",filename.latin1());

  saveAsXML(file,root_tree,0);

  file.device()->flush();
  file.device()->close();

  //delete file;
  }
}

void QTreeMapArea::xmlwalker(QDomElement docElem,int level){
  QDomNode n = docElem.firstChild();
  while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( !e.isNull() ) { // the node was really an element.
	cout << "tagname=" << e.tagName() << "name=" << e.attribute("name") << endl;
	xmlwalker(e,level+1);
      }
      n = n.nextSibling();
  }
}

void QTreeMapArea::loadFromXML(QString filename){
  QDomDocument doc(filename);

  QFile file(filename);
  if(!file.open(IO_ReadOnly)){
    return;
  }
  if(!doc.setContent(&file)){
    file.close();
    return;
  }
  file.close();

  QDomElement docElem = doc.documentElement();

  //  xmlwalker(docElem,0);

  
  //  return;

  QDomElement root_elem=doc.documentElement();
  xml_treemap_window=new QXmlTreeMapWindow();
  xml_treemap_window->makeWidgets();
  xml_treemap_window->getArea()->setOptions(options);
  xml_treemap_window->getArea()->setTreeMap((Object *)(new QDomElement(root_elem)));

}

void QTreeMapArea::loadFromXML(){
  QString filename=QFileDialog::getOpenFileName("treemap.xml", "XML (*.xml)" , this);

  if(!filename.isEmpty()){
    loadFromXML(filename);
  }
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
  dont_draw_xyd=1;
  dont_descend_xyd=1;
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
  maxlevels=3;
  piemap=FALSE;
  draw_pie_lines=FALSE;
  draw_hyper_lines=TRUE;
  area_is=AREA_IS_TOTALITEMS;
  modify_tree=FALSE;
  path_separator=QString("/");
  calc_nodesize=CALCNODE_IFEMPTY;

  select_color=QColor(255,200,200);
  match_color=QColor(100,200,230);

  scheme_list=NULL;
}

