/*
 *   File name:	qtreemaparea_squarify.cpp
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: 
 *
 */

/*
  the ideas behind the algorithms of qtreemaparea_squarify.cpp
  have been taken/inspired from the Document stm.pdf,
  "Squarified Treemaps" by Mark Bruls, Kees Huizing and Jarke J. van Wijk
  from the SequoiaView Homepage  http://www.win.tue.nl/sequoiaview/
  Email: {keesh,vanwijk,sequoia}@win.tue.nl
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


//#define DEBUG_SQR 1

// code for squarified treemaps

ObjList *QTreeMapArea::sortedList(Object *dutree){

  ObjList *slist=new ObjList();

  Object *child=firstChild(dutree);
  bool dotentry_flag=FALSE;
  while(child!=NULL){
    //printf("appending %s\n",fullName(child).latin1());

    if(totalSize(child)!=0){
      slist->append((KDirInfo *)child);
    }

    child=nextChild(child);
    if(child==NULL && dotentry_flag==FALSE){
      dotentry_flag=TRUE;
      Object *dotentry=sameLevelChild(dutree);
      if(dotentry){
	child=firstChild(dotentry);
      }
    }
  }

  slist->sort();

  //printf("list sorted!\n");

  return slist;
}


void QTreeMapArea::squarifyTree(Object *dutree,int x0,int y0,int xd0,int yd0, bool direction, int level,Cushion *cushion,int fx,int fy,int findmode){
  NOT_USED(direction);

  ObjList *sorted_list = sortedList(dutree);

  int len=sorted_list->count();
  int xd=xd0;
  int yd=yd0;

  if(options->show_inodes){
    float wasted_space=sum_list(sorted_list,0,len-1);

    float total=(float)totalSize(dutree);
    float percent;
    if(total==0){
      percent=1.0;
    }
    else{
      percent=((float)wasted_space)/(float)total;
    }


  if(xd0<yd0){
    xd=xd0;
    yd=(int)(((float)yd0)*percent);
  }
  else{
    yd=yd0;
    xd=(int)(((float)xd0)*percent);
  }
  }
#ifdef DEBUG_SQR
  printf("new list: [ ");
  printList(sorted_list,0,len-1);
  printf(" ]\n");
#endif

  if(len>0){
    int s=options->border_step;
    squarifyList(sorted_list,0,0,-1,x0+s,y0+s,xd-s,yd-s,0,level,cushion,fx,fy,findmode);
  }
  //  printf(" end sqr. tree\n");
}


void QTreeMapArea::printList(ObjList *slist,int i1,int i2){
  if(i2>=i1 && i1!=-1 && i2!=-1){
    float forget=sum_list(slist,i1,i2,TRUE);
    NOT_USED(forget);
  }
}


void QTreeMapArea::squarifyList(ObjList *slist,int ci,int sri,int ri,int x0,int y0,int xd0,int yd0, bool bogus_direction, int level,Cushion *cushion,int fx,int fy,int findmode){
  int w;
  bool direction=HORIZONTAL;

  NOT_USED(bogus_direction);

  if(xd0<options->dont_draw_xyd || yd0<options->dont_draw_xyd){
    return;
  }

#define isEmptyRow(i1,i2) ((i1!=-1 && i2!=-1 && i2>=i1) ? FALSE : TRUE )
#if 0
  if( ci==slist->count()-1 && isEmptyRow(sri,ri)){
    // last element in list
    layoutRow(slist,ci,ci,x0,y0,xd0,yd0,direction,0,0,level,cushion,fx,fy,findmode);
    printf(" end recursion\n");
    return;
  }
#endif

  if(ci==(int)slist->count()-1){
    layoutRow(slist,sri,ci,x0,y0,xd0,yd0,direction,0,0,level,cushion,fx,fy,findmode);
    return;
  }
  if(xd0>yd0){
    // fill verikal space
    direction=VERTIKAL;
    w=yd0;
  }
  else{
    direction=HORIZONTAL;
    w=xd0;
  }
#ifdef DEBUG_SQR
  printf("squarifyList: ci=%d,,sri=%d,ri=%d,dir=%d c=%s\n",ci,sri,ri,direction,fullName((Object *)slist->at(ci)).latin1());
  printf("children: [ ");
  printList(slist,ci,slist->count()-1);
  printf("]\n");
  printf("row: [ ");
  printList(slist,sri,ri);
  printf(" ] \n");
#endif
  //  if(worst_aspect(clist,0,len(clist)-1,

  //  Object *c=slist->at(ci);

  //  if(worst_aspect(slist,sri,ri,w)<=worst_aspect(slist,sri,ri+1,w)){

  float aspect1=worst_aspect(slist,sri,ri,w);
  float aspect2=worst_aspect(slist,sri,ri+1,w);
#if 0
  if( sum_list(slist,sri,slist->count()-1)==0 ){
    printf("abort divzero\n");
    return;
  }
#endif
  if(aspect2==-1){
    printf("ASPECT FAILURE\n");
    exit(0);
  }

  if(/* ci<(int)(slist->count()-1)  && */ aspect1==-1 || aspect1>=aspect2){
    squarifyList(slist,ci+1,sri,ri+1,x0,y0,xd0,yd0,0,level,cushion,fx,fy,findmode);
  }
  else{
    //    printf("before layout sri=%d ci=%d ri=%d\n",sri,ci,ri);
    //    int w2=(sum_list(slist,sri,ri)*w)/sum_list(slist,sri,slist->count()-1);
    //layoutRow(slist,sri,ri,x0,y0,xd0,yd0,direction,w,w2);
    if(direction==VERTIKAL){
      int w2=(int)((sum_list(slist,sri,ri)*xd0)/sum_list(slist,sri,slist->count()-1));
      layoutRow(slist,sri,ri,x0,y0,w2,yd0,direction,w,w2,level,cushion,fx,fy,findmode);
      squarifyList(slist,ci,ci,ci-1,x0+w2,y0,xd0-w2,yd0,0,level,cushion,fx,fy,findmode);
    }
    else{
      int w2=(int)((sum_list(slist,sri,ri)*yd0)/sum_list(slist,sri,slist->count()-1));
      layoutRow(slist,sri,ri,x0,y0,xd0,w2,direction,w,w2,level,cushion,fx,fy,findmode);
      squarifyList(slist,ci,ci,ci-1,x0,y0+w2,xd0,yd0-w2,0,level,cushion,fx,fy,findmode);
    }
  }
}

void QTreeMapArea::layoutRow(ObjList *slist,int sri,int ri,int x0,int y0,int xd0,int yd0,bool direction, int w,int w2, int level,Cushion *cushion,int fx,int fy,int findmode){
  //  printf("layout: %s\n",fullName((Object *)(slist->at(sri))).latin1());

  NOT_USED(w);
  NOT_USED(w2);

#ifdef DEBUG_SQR
  printf("layout: [ ");
  printList(slist,sri,ri);
  printf(" ]\n");
#endif
  float total=sum_list(slist,sri,ri);
  
  //  direction=!direction;

  if(xd0<yd0){
    direction=HORIZONTAL;
  }
  else{
    direction=VERTIKAL;
  }
#if 0
  if(!findmode){
    paintEntry(x0,y0,xd0,yd0,QString("Blah"),direction,level,QColor(200,0,0),PM_FLAT,NULL);
  }
#endif

#define LW 0

  x0+=LW;
  y0+=LW;
  xd0-=LW;
  yd0-=LW;

  int i=sri;
  float xd=0.0;
  float yd=0.0;
  float x=x0;
  float y=y0;

  while(i<=ri){
    Object *child=(Object *)(slist->at(i));

    float percent_size=((float)totalSize(child)/(float)total);
  
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

    drawDuTree(child,(int)x,(int)y,(int)xd,(int)yd,0,level,cushion,fx,fy,findmode);
    
    if(direction==HORIZONTAL){
      // horizontal
      y=y+yd;
    }
    else{
      x=x+xd;
    }
    i=i+1;
  } //while
}

float QTreeMapArea::sum_list(ObjList *slist,int i1,int i2,bool print_it){
   float sum=0;
   for(int i=i1;i<=i2;i++){
     float size=(float)totalSize((Object *)(slist->at(i)));
     sum+=size;
       if(print_it){
	 printf("%ld ",(long)size);
       }
   }
   return sum;
 }


float QTreeMapArea::worst_aspect(ObjList *sorted_list,int i1,int i2,int width){
  // returns the worst aspect from sorted_list
  // starting at index i1 to (inclusive) i2

  float worst;
  float aspect=0.0;
  float aspect2=0.0;

  //if(i2<0 || i1<0 || i2-i1<0){
  if(isEmptyRow(i1,i2)){
    //worst=-1;
    worst=-1;
  }
  else{

#if 0
    int s=sum_list(sorted_list,i1,i2);
    int rp=totalSize((Object *)(sorted_list->at(i1)));
    int rm=totalSize((Object *)(sorted_list->at(i2)));
    worst=MAX( ((float)(width*width)*rp)/(float)(s*s) , ((float)(s*s)/(float)((width*width)*rm)));
#endif

#define CHECKPOS(a,b) if(b<0){ printf("%s is negative!\n",a); }

    float last_size=(float)totalSize((Object *)(sorted_list->at(i2)));

    float total_size=sum_list(sorted_list,i1,i2);
    float whole_size=sum_list(sorted_list,i1,sorted_list->count()-1);

    CHECKPOS("last",last_size);
    CHECKPOS("total",total_size);
    CHECKPOS("whole",whole_size);
    //    CHECKPOS("last",last_size);

    if(total_size==0 || whole_size==0){
      printf("abort divzero\n");
      return -1;
    }

    float percent=((float)last_size)/(float)total_size;

    float h=(((float)width)*percent);

    float w2=(total_size*width)/whole_size;

    CHECKPOS("percent",percent);
    CHECKPOS("h",h);
    CHECKPOS("w2",w2);

    aspect=((float)h)/(float)w2;

    CHECKPOS("aspect",aspect);

    aspect2=((float)(last_size*sum_list(sorted_list,i1,sorted_list->count()-1)))/(float)sum_list(sorted_list,i1,i2);

    worst=MAX(aspect,1.0/aspect);
  }
#ifdef DEBUG_SQR
  printf("worst: %d %d w=%d list=[ ",i1,i2,width);
  printList(sorted_list,i1,i2);
  printf(" ] = %f (a1=%f,a2=%f)\n",worst,aspect,aspect2);
#endif

  return worst;
}


//int ObjList::compareItems(Object *o1,Object *o2){
int ObjList::compareItems(QCollection::Item o1, QCollection::Item o2){
  if( (((KDirInfo *)o1)->totalSize()) >  (((KDirInfo *)o2)->totalSize()) ){
    return -1;
  }
  else if( (((KDirInfo *)o1)->totalSize()) < (((KDirInfo *)o2)->totalSize()) ){
    return 1;
  }
  else{
       return 0;
     }
}


#if 0
void squarifyList(clist,rlist,x0,y0,xd0,yd0){
  if(xd0>yd0){
    // fill verikal space
    direction=VERTIKAL;
    w=yd0;
  }
  else{
    direction=HORIZONTAL;
    w=xd0;
  }

  //  if(worst_aspect(clist,0,len(clist)-1,

  c=clist[0];

  if(worst(rlist,0,w)<=worst(rlist,c,w)){
    squarifyList()
}

void STM(dutree,x0,y0,xd0,yd0){


}
#endif

