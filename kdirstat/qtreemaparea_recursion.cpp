/*
 *   File name:	qtreemaparea_recursion.cpp
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
  the ideas behind the Cushion Treemaps (method CTM and shading PM_CUSHION)
  have been taken/inspired from the Document ctm.pdf,
  "Cushion Treemaps" by Jarke J. van Wijk and Huub van de Wetering
  from the SequoiaView Homepage  http://www.win.tue.nl/sequoiaview/
  Email: {vanwijk,wstahw,sequoia}@win.tue.nl
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


void QTreeMapArea::drawTreeMap(Object *dutree){
  int x0=0;
  int y0=0;
  int xd0=options->paint_size_x;
  int yd0=options->paint_size_y;

  offscreen.resize(options->paint_size_x,options->paint_size_y);
  color_index=0;

  painter->begin(&offscreen);
  //painter->setWindow(x0,y0,xd0,yd0);
  painter->setFont(QFont( "times",10));

  painter->eraseRect(0,0,options->paint_size_x,options->paint_size_y);

  last_shaded=NULL;
  next_shaded=NULL;

  Cushion *cushion=NULL;

  if(TRUE /* options->paintmode==PM_CUSHION || options->paintmode==PM_HIERARCH_CUSHION || options->paintmode==PM_HIERARCH2_CUSHION
     || options->paintmode==PM_HIERARCH3_CUSHION
     || options->paintmode==PM_HIERARCH4_CUSHION
     || options->paintmode==PM_HIERARCH5_CUSHION
     || options->paintmode==PM_HIERARCH_TEST_CUSHION */){
    cushion=new Cushion(xd0,yd0,options->sequoia_h,options->sequoia_f);
  }

  if(options->paintmode==PM_CUSHION){
    CTM(dutree,options->start_direction,cushion);
  }
  else{
    drawDuTree(dutree,x0,y0,xd0,yd0,options->start_direction,0,cushion);
  }

  delete cushion;

  //  printf("END OF RECURSION\n");
    painter->end();

    this->update();
    this->repaint();
    //    graph_widget->repaint();
}


void QTreeMapArea::drawDuTree(Object *dutree, int x0,int y0,int xd0, int yd0, bool direction, int level,Cushion *cushion,int fx,int fy,int findmode){
  QString node_name=fullName(dutree);
  Object *sub_nodes=firstChild(dutree);
  int node_totalsize=totalSize(dutree);
  Cushion *c=NULL;

  //    printf("QTreeMapArea::drawDuTree(%s,%d,%d,%d,%d,dir=%d,level=%d) %d\n",node_name.latin1(),x0,y0,xd0,yd0,direction,level,node_totalsize);

  if(options->dont_draw_xyd==-1 || (xd0>=options->dont_draw_xyd && yd0>=options->dont_draw_xyd)){
    if(findmode==FIND_MATCH){
      if(find_regexp.match(node_name)!=-1){
	found_kfileinfo=dutree;
	//	printf("found match: %s\n",node_name.latin1());
      }
    }
    /* else */ if((fx>=0 && fy>=0) /* && findmode!=FIND_MATCH */){
    // search mode
    if( ( x0<=fx && fx<=(x0+xd0) ) && ( ( y0<=fy && fy<=(y0+yd0)) ) ){
      // mouse coord are inside this entries coordinates
      found_kfileinfo=dutree;
      if(findmode==FIND_FIRSTDIR && dutree!=root_tree){
	// we have found the first directory with this coordinates
	//if(options->dynamic_shading==FALSE){
	  return;
	  //}
      }
    }
    else{
      if(options->dynamic_shading==FALSE){
	// don't descend into this part of tree
	return;
      }
    }
  }

  if(TRUE){
    // not in search mode
    if(cushion!=NULL){
      c=new Cushion(*cushion);
      if(!(dutree==root_tree || parentNode(dutree)==root_tree)){
	cushion_AddRidge(c->r[direction][0],
			 c->r[direction][1],
			 c->h,
			 c->s[direction][1],
			 c->s[direction][2]);
      }
      if(TRUE /* options->paintmode==PM_HIERARCH_CUSHION ||
	 options->paintmode==PM_HIERARCH2_CUSHION
	 || options->paintmode==PM_HIERARCH3_CUSHION
	 || options->paintmode==PM_HIERARCH4_CUSHION
	 || options->paintmode==PM_HIERARCH5_CUSHION */){
	float cxd=xd0/2;
	float cyd=yd0/2;
	int cx=(int)(x0+cxd);
	int cy=(int)(y0+cyd);
	float ncxd,ncyd;

	if((c->cx0)<=cx){
	  ncxd=cxd-(cxd*(options->hc_factor));
	}
	else{
	  ncxd=(cxd*(options->hc_factor))+cxd;
	}

	if((c->cy0)<=cy){
	  ncyd=cyd-(cyd*(options->hc_factor));
	}
	else{
	  ncyd=(cyd*(options->hc_factor))+cyd;
	}

	int ncx=(int)(x0+ncxd);
	int ncy=(int)(y0+ncyd);

	//printf("HC: hc_f=%f, cx=%d,cy=%d, ncx=%d, ncy=%d\n",options->hc_factor,
	//	       cx,cy,ncx,ncy);
	c->cx0=ncx;
	c->cy0=ncy;
	c->ncxd=ncxd;
	c->ncyd=ncyd;
      }
    }
    else{
      c=NULL;
    }
  }

  if((options->draw_mode==DM_FILES || options->draw_mode==DM_BOTH )&& isLeaf(dutree) /* && (fx==-1 && fy==-1) */ ){
	QColor basecolor;
	basecolor=getBaseColor(node_name);
	//basecolor=QColor(rotate_colors[color_index]);
	//getNextRotatingColorIndex();

	int pmode;
	if((fx>=0 && fy>=0)){
	  if(findmode==FIND_SELECTION){
	    if(dutree==found_kfileinfo){
	      if(selected_list->containsRef((KDirInfo *)dutree)){
		printf("setting selection\n");
		QColor foundcolor=options->select_color;
		paintEntry(x0,y0,xd0,yd0,node_name,direction,level,foundcolor,options->paintmode,c);
	      }
	      else{
		printf("clearing selection\n");
		QColor foundcolor=getBaseColor(node_name);
		paintEntry(x0,y0,xd0,yd0,node_name,direction,level,foundcolor,options->paintmode,c);
	      }
	  }
	    
	  }
	  else{
	  if(options->dynamic_shading && (next_shaded==dutree || next_shaded==NULL)){
	      QPaintDevice *pd=painter->device();
	      painter->end();
	      painter->begin(&offscreen);
	      paintEntry(x0,y0,xd0,yd0,node_name,direction,level,basecolor,PM_SQUARE_CUSHION,c);
	      painter->end();
	      painter->begin(pd);
	      next_shaded=nextChild(next_shaded);
	  }
	  // when searching, only draw outline
	    pmode=PM_OUTLINE;
	    if(dutree==found_kfileinfo){
#if 0
	      QPaintDevice *pd=painter->device();
	      painter->end();
	      painter->begin(&offscreen);
	      paintEntry(x0,y0,xd0,yd0,node_name,direction,level,basecolor,PM_SQUARE_CUSHION,c);
	      painter->end();
	      painter->begin(pd);
#endif
	      paintEntry(x0,y0,xd0,yd0,node_name,direction,level,options->highlight_frame_col,pmode,c);
	  }
	  } // end else selection
	}
	else if(findmode==FIND_MATCH){
	  if(dutree==found_kfileinfo){
	    QColor foundcolor=options->match_color;
	    paintEntry(x0,y0,xd0,yd0,node_name,direction,level,foundcolor,options->paintmode,c);	    
	  }
	}
	else{
	  // really draw this entry
	  pmode=options->paintmode;
	  if(selected_list->containsRef((KDirInfo *)dutree)){
		printf("setting selection\n");
		QColor fcolor=options->select_color;
		paintEntry(x0,y0,xd0,yd0,node_name,direction,level,fcolor,options->paintmode,c);
	  }
	  else{
	  paintEntry(x0,y0,xd0,yd0,node_name,direction,level,basecolor,pmode,c);
#if 0
	  QPaintDevice *pd=painter->device();
	  painter->end();
	  painter->begin(this);
	  paintEntry(x0,y0,xd0,yd0,node_name,direction,level,basecolor,pmode,c);
	  painter->end();
	  painter->begin(pd);
#endif
	  }
	}

      }
      else if((options->draw_mode==DM_DIRS || options->draw_mode==DM_BOTH) && ( isNode(dutree) || isSameLevelChild(dutree))) {
#define MDEPTH 6
	//QColor basecolor=QColor(rotate_colors[color_index]);
	QColor basecolor=QColor(dirBaseColor);
	float i=((float)(MDEPTH-level))/((float)MDEPTH);
	QColor dircolor=QColor((int)(basecolor.red()*i),
			     (int)(basecolor.green()*i),
			     (int)(basecolor.blue()*i));
	//getNextRotatingColorIndex();

	//Object *dirinfo=(Object *)dutree;

	int pmode;
	if(fx>=0 && fy>=0){
	  // draw outline in search mode
	  pmode=PM_OUTLINE;
	  if(dutree==found_kfileinfo){
	    paintEntry(x0,y0,xd0,yd0,node_name,direction,level,options->highlight_frame_col,pmode,c);
	  }
	}
	else if(findmode==FIND_MATCH){
	  // leave it
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

  if(options->squarify==TRUE){
    squarifyTree(dutree,x0,y0,xd0,yd0,0,level,c,fx,fy,findmode);
  }
  else{
      float x=(float)(x0 + options->border_step);
      float y=(float)(y0 + options->border_step);
      float xd=0.0;
      float yd=0.0;
      float w=0.0;
      float xd0s=xd0 - (options->border_step);
      float yd0s=yd0 - (options->border_step);

      if(c!=NULL){
	// cushion mode
	w=( c->r[direction][1] - c->r[direction][0] )/((float)node_totalsize);
      }
      for(Object *subtree=sub_nodes;subtree!=NULL;subtree=nextChild(subtree)){
	int subnode_size=totalSize(subtree);
	  if(subnode_size==0){
	    // we do not descend in directories with 0 size
	  }
	  else{
	    float percent_size=((float)subnode_size)/((float)node_totalsize);
	    
	    if(direction==HORIZONTAL){
	      // horizontal
	      xd=(float)xd0s;
	      yd=(float)(((float)yd0s)*percent_size);
	    }
	    else{
	      // vertikal
	      xd=(float)(((float)xd0s)*percent_size);
	      yd=(float)yd0s;
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

      if( sameLevelChild(dutree)!=NULL ){
	Object *dotentry=sameLevelChild(dutree);
	int subnode_size=totalSize(dotentry);
	    float percent_size=((float)subnode_size)/((float)node_totalsize);
	    
	    if(direction==HORIZONTAL){
	      // horizontal
	      xd=(float)xd0s;
	      yd=(float)(((float)yd0s)*percent_size);
	    }
	    else{
	      // vertikal
	      xd=(float)(((float)xd0s)*percent_size);
	      yd=(float)yd0s;
	    }
	    bool subdirection=direction;
	    
	    int sw=options->step_width;

	    drawDuTree(dotentry,(int)x,(int)y,(int)(xd-sw),(int)(yd-sw),subdirection,level,c,fx,fy,findmode);
      }
  }
  delete c;
  }
}


void QTreeMapArea::CTM(Object *tree,bool direction,Cushion *cushion){
  Cushion *c=new Cushion(*cushion);
  //if(parentNode(tree)!=NULL){
  if(!(tree==root_tree || parentNode(tree)==root_tree)){
    cushion_AddRidge(c->r[direction][0],
			 c->r[direction][1],
			 c->h,
			 c->s[direction][1],
			 c->s[direction][2]);
  }
  if(isLeaf(tree)){
    QColor basecolor=QColor(255,0,0);
    paintEntry(0,0,0,0,fullName(tree),direction,0,basecolor,PM_CUSHION,c);
  }
  else{
    if(direction==HORIZONTAL){
      direction=VERTIKAL;
    }
    else{
      direction=HORIZONTAL;
    }
    float w=( c->r[direction][1] - c->r[direction][0] )/((float)totalSize(tree));
    
    Object *child=firstChild(tree);
    bool dotentry_flag=FALSE;
    while(child!=NULL){
      	      c->r[direction][1]=c->r[direction][0] + ( w*((float)totalSize(child)) );
	      c->h=c->h*c->f;
	      CTM(child,direction,c);
	      c->r[direction][0]=c->r[direction][1];
	      child=nextChild(child);
#if 1
	      if(child==NULL && dotentry_flag==FALSE){
		dotentry_flag=TRUE;
		Object *dotentry=sameLevelChild(tree);
		if(dotentry){
		  child=firstChild(dotentry);
		}
	      }
#endif
    }
  }
  delete c;
}


void QTreeMapArea::cushion_AddRidge(float x1,float x2,float h,float& s1,float& s2){
  s1=s1+4*h*(x2+x1)/(x2-x1);
  s2=s2-4*h/(x2-x1);
}


Cushion::Cushion(int xd,int yd,float sh,float sf){
  r[HORIZONTAL][0]=0.0;
  r[HORIZONTAL][1]=(float)xd;
  r[VERTIKAL][0]=0.0;
  r[VERTIKAL][1]=(float)yd;

  s[HORIZONTAL][1]=0.0;
  s[HORIZONTAL][2]=0.0;
  s[VERTIKAL][1]=0.0;
  s[VERTIKAL][2]=0.0;

  h=sh;
  f=sf;

  cx0=xd;
  cy0=yd;
}

