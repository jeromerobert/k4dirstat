/*
 *   File name:	qlistviewtreemaparea.cpp
 *   Summary:	
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
#include "qlistviewtreemap.h"
#include <qmainwindow.h>
//#include <qdom.h>


//#define _dp(x) ((Object *)(new QDomElement(x)))

QListViewTreeMapArea::QListViewTreeMapArea(int col_name,int col_size,QWidget *parent) : QTreeMapArea(parent) {
  useColumns(col_name,col_size);
}

void QListViewTreeMapArea::useColumns(int col_name,int col_size){
  column_name=col_name;
  column_size=col_size;

}

QString QListViewTreeMapArea::shortName(Object *node){
  QListViewItem *kdi_node=(QListViewItem *)node;

  return kdi_node->text(column_name);
}

QString QListViewTreeMapArea::fullName(Object *node){
  //  QListViewItem *kdi_node=(QListViewItem *)node;

  //  QString name=kdi_node->text(column_name);

  return findFullName(node);
}

int QListViewTreeMapArea::thisDirItems(Object *node){
  //  QListViewItem *kdi_node=(QListViewItem *)node;
  int count=0;
  
  if(isNode(node)){
    Object *child=firstChild(node);
    bool dotentry_flag=FALSE;

    while(child!=NULL){
      count++;
      child=nextChild(child);
	      if(child==NULL && dotentry_flag==FALSE){
		dotentry_flag=TRUE;
		Object *dotentry=sameLevelChild(node);
		if(dotentry){
		  child=firstChild(dotentry);
		}
	      }
     }
  }
  else{
  }

  return count;
}
#if 1
void QListViewTreeMapArea::printXmlInfo(Object *node){
  QListViewItem *kdi_node=(QListViewItem *)node;

  printf("NODE:n=%x name=%s size=%s\n",(int)kdi_node,kdi_node->text(column_name).latin1(),kdi_node->text(column_size).latin1());

}
#endif

#if 0
QListViewItem QListViewTreeMapArea::findElement(QDomNode n){
  QListViewItem elem;

  while(!n.isNull() && elem.isNull()){
    elem=n.toElement();
    n=n.nextSibling();
  }
  return elem;
}
#endif

Object *QListViewTreeMapArea::firstChild(Object *node){
  //    printf("firstChild: ");
  //  printXmlInfo(node);
  QListViewItem *kdi_node=(QListViewItem *)node;

  QListViewItem *n=kdi_node->firstChild();
  
  return (Object *)n;
}


asize QListViewTreeMapArea::totalSize(Object *node){
  //  printf("totalsize: ");
  // printXmlInfo(node);
  QListViewItem *kdi_node=(QListViewItem *)node;

  QString sizeattr=kdi_node->text(column_size);
  asize size=0.0;
  if(sizeattr=="" || sizeattr.isNull() || sizeattr.isEmpty()
     || (options->calc_nodesize==CALCNODE_ALWAYS && isNode(node))){
    //    printf("calc. size\n");
    if(isNode(node)){
      size=calcTotalSize(node);
      //printf("calc size: %f\n",(float)size);
      if(options->modify_tree==TRUE){
	// modify
      }
    }
    else{
      printf("ERROR! can't calc. size for leaf!\n");
      size=0.0;
    }
  }
  else{
    size=(asize)sizeattr.toFloat();
  }

  return size;
}

int QListViewTreeMapArea::totalItems(Object *node){
  NOT_USED(node);
  //  QListViewItem *kdi_node=(QListViewItem *)node;

  //return kdi_node->totalItems();
  return 0;
}

bool QListViewTreeMapArea::isLeaf(Object *node){
  //printXmlInfo(node);
  QListViewItem *kdi_node=(QListViewItem *)node;

  QListViewItem *n=kdi_node->firstChild();

  if(n==NULL){
    return TRUE;
  }
  else{
    return FALSE;
  }
}

bool QListViewTreeMapArea::isNode(Object *node){
  //printXmlInfo(node);
  //QListViewItem *kdi_node=(QListViewItem *)node;

  return !isLeaf(node);
}

bool QListViewTreeMapArea::isSameLevelChild(Object *node){
  NOT_USED(node);
  //QListViewItem *kdi_node=(QListViewItem *)node;

  return FALSE;
}

Object *QListViewTreeMapArea::nextChild(Object *node){
  //    printf("nextChild: ");
  //  printXmlInfo(node);
  QListViewItem *kdi_node=(QListViewItem *)node;

  QListViewItem *n=kdi_node->nextSibling();
  
  return (Object *)n;
}

Object *QListViewTreeMapArea::sameLevelChild(Object *node){
  NOT_USED(node);
  //QListViewItem *kdi_node=(QListViewItem *)node;

  return NULL;
}

Object *QListViewTreeMapArea::parentNode(Object *node){
  QListViewItem *kdi_node=(QListViewItem *)node;

  QListViewItem *n=kdi_node->parent();

  return (Object *)n;

}

QString QListViewTreeMapArea::tellUnit(asize size){
  QString str;
  if((float)((int)size)==(float)size){
    str.sprintf("%d",(int)size);
  }
  else{
    str.sprintf("%f",(float)size);
  }

#if 0
  if(size<1024){
    str.sprintf("%d bytes",(int)size);
  }
  else if(size<(1024*1024)){
    str.sprintf("%.2f kB",((float)size)/(1024.0));
  }
  else if(size<(1024*1024*1024)){
    str.sprintf("%.2f MB",((float)size)/(1024*1024));
  }
  else {
    str.sprintf("%.2f GB",((float)size)/(1024*1024*1024));
  }
#endif
  return str;
}

void QListViewTreeMapArea::dirChange(Object *node){
  NOT_USED(node);
#if 0
  //  printf("CALLBACK QListViewTreeMapArea\n");
  emit changedDirectory(node);
#endif
}
void QListViewTreeMapArea::directoryUp(){
  //  printf("CALLBACK QListViewTreeMapArea\n");
  ((QTreeMapArea *)this)->directoryUp();

}

void QListViewTreeMapArea::saveAsBitmap(){
  //printf("CALLBACK QListViewTreeMapArea\n");
  ((QTreeMapArea *)this)->saveAsBitmap();

}

void QListViewTreeMapArea::saveAsXML(){
  //printf("CALLBACK QListViewTreeMapArea\n");
  ((QTreeMapArea *)this)->saveAsXML();

}

void QListViewTreeMapArea::zoomIn(){
  //printf("CALLBACK QListViewTreeMapArea\n");
  ((QTreeMapArea *)this)->zoomIn();

}

void QListViewTreeMapArea::zoomOut(){
  //printf("CALLBACK QListViewTreeMapArea\n");
  ((QTreeMapArea *)this)->zoomOut();

}
/*
void QListViewTreeMapArea::highlighted(Object *high){
  printf("CALLBACK QListViewTreeMapArea\n");
  ((QTreeMapArea *)this)->highlighted(high);

}

*/
