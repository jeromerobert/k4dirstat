/*
 *   File name:	qxmltreemaparea.cpp
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
#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>
#include "qxmltreemap.h"
#include <qmainwindow.h>
#include <qdom.h>


#define _dp(x) ((Object *)(new QDomElement(x)))

QXmlTreeMapArea::QXmlTreeMapArea(QWidget *parent) : QTreeMapArea(parent) {
}

QString QXmlTreeMapArea::shortName(Object *node){
  //  printXmlInfo(node);
  QDomElement *kdi_node=(QDomElement *)node;

  return kdi_node->attribute("name");
}

QString QXmlTreeMapArea::fullName(Object *node){
  //  printXmlInfo(node);
  QDomElement *kdi_node=(QDomElement *)node;

  QString name=kdi_node->attribute("name");
  //  printf("xml:name=%s\n",name.latin1());
  return name;
}

int QXmlTreeMapArea::thisDirItems(Object *node){
  //  QDomElement *kdi_node=(QDomElement *)node;
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

void QXmlTreeMapArea::printXmlInfo(Object *node){
  QDomElement *kdi_node=(QDomElement *)node;

  printf("NODE:n=%x tag=%s name=%s value=%s attr=%s\n",(int)kdi_node,kdi_node->tagName().latin1(), \
	 kdi_node->nodeName().latin1(), \
	 kdi_node->nodeValue().latin1(), \
	 kdi_node->attribute("name").latin1());
}

QDomElement QXmlTreeMapArea::findElement(QDomNode n){
  QDomElement elem;

  while(!n.isNull() && elem.isNull()){
    elem=n.toElement();
    n=n.nextSibling();
  }
  return elem;
}

Object *QXmlTreeMapArea::firstChild(Object *node){
  //  printf("firstChild: ");
  //printXmlInfo(node);
  QDomElement *kdi_node=(QDomElement *)node;

  QDomNode n=kdi_node->firstChild();
  QDomElement elem=findElement(n);

    
  //QDomElement elem=(kdi_node->firstChild().toElement());//.toElement());

  if(elem.isNull()){
    return NULL;
  }
  else{
    return _dp(elem);
  }
}



int QXmlTreeMapArea::totalSize(Object *node){
  //printXmlInfo(node);
  QDomElement *kdi_node=(QDomElement *)node;

  return kdi_node->attribute("size","0").toInt();
}
int QXmlTreeMapArea::totalItems(Object *node){
  NOT_USED(node);
  //  QDomElement *kdi_node=(QDomElement *)node;

  //return kdi_node->totalItems();
  return 0;
}

bool QXmlTreeMapArea::isLeaf(Object *node){
  //printXmlInfo(node);
  QDomElement *kdi_node=(QDomElement *)node;

  QDomNode n=kdi_node->firstChild();
  QDomElement elem=findElement(n);

  if(elem.isNull()){
    //    printf("isleaf\n");
    return TRUE;
  }
  return FALSE;
}

bool QXmlTreeMapArea::isNode(Object *node){
  //printXmlInfo(node);
  //QDomElement *kdi_node=(QDomElement *)node;

  return !isLeaf(node);
}

bool QXmlTreeMapArea::isSameLevelChild(Object *node){
  NOT_USED(node);
  //QDomElement *kdi_node=(QDomElement *)node;

  return FALSE;
}

Object *QXmlTreeMapArea::nextChild(Object *node){
  //  printf("nextChild: ");
  //  printXmlInfo(node);
  QDomElement *kdi_node=(QDomElement *)node;

  QDomNode n=kdi_node->nextSibling();
  QDomElement elem=findElement(n);

  if(elem.isNull()){
    return NULL;
  }
  else{
    return _dp(elem);
  }

}

Object *QXmlTreeMapArea::sameLevelChild(Object *node){
  NOT_USED(node);
  //QDomElement *kdi_node=(QDomElement *)node;

  return NULL;
}

Object *QXmlTreeMapArea::parentNode(Object *node){
  QDomElement *kdi_node=(QDomElement *)node;

  QDomNode n=kdi_node->parentNode();
   QDomElement elem=findElement(n);

  if(elem.isNull()){
    return NULL;
  }
  else{
    return _dp(elem);
  }

}

QString QXmlTreeMapArea::tellUnit(int size){
  QString str;
  if(size<1024){
    str.sprintf("%d bytes",size);
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

  return str;
}

void QXmlTreeMapArea::dirChange(Object *node){
  NOT_USED(node);
#if 0
  //  printf("CALLBACK QXmlTreeMapArea\n");
  emit changedDirectory(node);
#endif
}
void QXmlTreeMapArea::directoryUp(){
  //  printf("CALLBACK QXmlTreeMapArea\n");
  ((QTreeMapArea *)this)->directoryUp();

}

void QXmlTreeMapArea::saveAsBitmap(){
  //printf("CALLBACK QXmlTreeMapArea\n");
  ((QTreeMapArea *)this)->saveAsBitmap();

}

void QXmlTreeMapArea::saveAsXML(){
  //printf("CALLBACK QXmlTreeMapArea\n");
  ((QTreeMapArea *)this)->saveAsXML();

}

void QXmlTreeMapArea::zoomIn(){
  //printf("CALLBACK QXmlTreeMapArea\n");
  ((QTreeMapArea *)this)->zoomIn();

}

void QXmlTreeMapArea::zoomOut(){
  //printf("CALLBACK QXmlTreeMapArea\n");
  ((QTreeMapArea *)this)->zoomOut();

}
/*
void QXmlTreeMapArea::highlighted(Object *high){
  printf("CALLBACK QXmlTreeMapArea\n");
  ((QTreeMapArea *)this)->highlighted(high);

}

*/
