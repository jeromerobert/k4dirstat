/*
 *   File name:	qtreemaparea_paint.cpp
 *   Summary:	Support classes for KDirStat
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
#include <stdio.h>
#include <unistd.h>
#include <qtimer.h>
#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>
#include "qtreemap.h"
#include <qmainwindow.h>


void QTreeMapArea::paintEntry(int x0, int y0, int xd, int yd,QString entry_name,bool direction,int level,const QColor &basecolor,int pmode,Cushion *c){

  NOT_USED(direction);
  NOT_USED(level);
#if 0
	      QPaintDevice *pd=painter->device();
	      painter->end();
	      painter->begin(this);
  paintEntry(x0,y0,xd,yd,entry_name,direction,level,basecolor,pmode,c);
  painter->end();
  painter->begin(pd);
#endif
#define MPIX 1

  if(xd<=MPIX || yd<=MPIX){
    if(xd<=MPIX){
      xd=MPIX;
    }
    if(yd<=MPIX){
      yd=MPIX;
    }
  }

  if(pmode==PM_OUTLINE){
    // draw highlighted frames in search mode
    mypen.setColor(basecolor);
    mypen.setWidth(options->highlight_frame_width);
      painter->setPen( mypen );
      painter->setBrush( Qt::NoBrush );

    painter->drawRect(x0,y0,xd,yd);
  }
  else if(pmode==PM_FLAT){
    painter->fillRect(x0,y0,xd,yd,basecolor);
  }
  else if(pmode==PM_IMAGES){
    painter->fillRect(x0,y0,xd,yd,basecolor);
    
    QPixmap *pic=new QPixmap(entry_name);
    if(pic && !(pic->isNull())){
      pic->resize(xd,yd);
      painter->drawPixmap(x0,y0,*pic,0,0);
    }
    delete pic;
  }
  else if(pmode==PM_HIERARCH_DIST_PYRAMID){
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	int maxdist_x,maxdist_y,dx,dy;

	if(w < c->ncxd){
	  maxdist_x=(int)c->ncxd;
	  dx=w;
	}
	else{
	  maxdist_x=xd - (int)c->ncxd;
	  dx=xd-w;
	}
	if(h < c->ncyd){
	  maxdist_y=(int)c->ncyd;
	  dy=h;
	}
	else{
	  maxdist_y=yd - (int)c->ncyd;
	  dy=yd-h;
	}
	float ix=((float)dx)/(float)maxdist_x;
	float iy=((float)dy)/(float)maxdist_y;
	float i=(ix+iy)/2.0;
	//float i=sqrt(ix*iy);

	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
  }
  else if(pmode==PM_HIERARCH_DIST_SIN_PYRAMID){
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	int maxdist_x,maxdist_y,dx,dy;

	if(w < c->ncxd){
	  maxdist_x=(int)c->ncxd;
	  dx=w;
	}
	else{
	  maxdist_x=xd - (int)c->ncxd;
	  dx=xd-w;
	}
	if(h < c->ncyd){
	  maxdist_y=(int)c->ncyd;
	  dy=h;
	}
	else{
	  maxdist_y=yd - (int)c->ncyd;
	  dy=yd-h;
	}
	float ix=sin(((float)dx)/(float)maxdist_x);
	float iy=sin(((float)dy)/(float)maxdist_y);
	float i=(ix+iy)/2.0;
	//float i=sqrt(ix*iy);

	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
  }
  else if(pmode==PM_HIERARCH_PYRAMID){
    float steps=16.0;
    float sxdl=((float)(c->cx0 - x0))/steps;
    //float sxdr=((float)(x0+xd - c->cx0))/steps;
    float sxdr=((float)(xd - c->ncxd))/steps;
    float sydo=((float)(c->cy0 - y0))/steps;
    float sydu=((float)(y0+yd - c->cy0))/steps;

    for(float s=0;s<=steps;s++){
      float i=((float)s+5)/(float)steps;
      QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			   check_int((int)(basecolor.green()*i)),
			   check_int((int)(basecolor.blue()*i)));
      int l=x0+(int)(sxdl*s);
      int o=y0+(int)(sydo*s);
      int r=x0+xd-(int)(sxdr*s);
      int u=y0+yd-(int)(sydu*s);

      painter->fillRect(l,
			o,
			r-l,
			u-o,
			newcol);

    }
#if 0
    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);
#endif
    
  }
  else if(pmode==PM_HIERARCH_CUSHION){
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float ix=sin(  ( ( (float)w ) / (float)( 2.0 * c->ncxd) )*3.14f);
	float iy=sin(  ( ( (float)h ) / (float)( 2.0 * c->ncyd) )*3.14f);
	float i=(ix+iy)/(2.0); // intensity as a sin in x/y direction
	if(isnan(i)){
	  i=0.0;
	}
	i=MIN(1.0,i);
	i=MAX(0.0,i);
	//printf("i=%f\n",i);
	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
	mypen.setColor(basecolor);
	mypen.setWidth( 1);
	painter->setPen( mypen );
#if 0
    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);
#endif

  }
  else if(pmode==PM_HIERARCH2_CUSHION){
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float ix=sin(  ( ( (float)w ) / (float)( 2.0 * c->ncxd) )*3.14f);
	float iy=sin(  ( ( (float)h ) / (float)( 2.0 * c->ncyd) )*3.14f);
	float i=(ix+iy)/(2.0); // intensity as a sin in x/y direction
	if(isnan(i)){
	  i=0.0;
	}
	i=MIN(1.0,i);
	i=MAX(0.3,i);
	//printf("i=%f\n",i);
	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
	mypen.setColor(basecolor);
	mypen.setWidth( 1);
	painter->setPen( mypen );
#if 0
    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);
#endif

  }
  else if(pmode==PM_HIERARCH3_CUSHION){
    float maxdist=sqrt( powf(xd,2.0) + powf(yd,2.0) );
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){

	float dist= sqrt( powf(c->ncxd - w,2.0) + powf(c->ncyd - h,2.0) );


	//float i=dist/maxdist;
	float i=1.0-(dist/maxdist);
	//	float i=maxdist/dist;
	if(isnan(i)){
	  i=0.0;
	}
	i=MIN(1.0,i);
	i=MAX(0.3,i);
	//printf("i=%f\n",i);
	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
	mypen.setColor(basecolor);
	mypen.setWidth( 1);
	painter->setPen( mypen );
#if 0
    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);
#endif

  }
  else if(pmode==PM_HIERARCH4_CUSHION){
    float maxdist=sqrt( powf(xd/2,2.0) + powf(yd/2,2.0) );
    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){

	float dist= sqrt( powf(c->ncxd - w,2.0) + powf(c->ncyd - h,2.0) );


	//float i=dist/maxdist;
	float i=1.0-(dist/maxdist);
	//	float i=maxdist/dist;
	if(isnan(i)){
	  i=0.0;
	}
	i=MIN(1.0,i);
	i=MAX(0.0,i);
	//printf("i=%f\n",i);
	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
	mypen.setColor(basecolor);
	mypen.setWidth( 1);
	painter->setPen( mypen );
#if 0
    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);
#endif

  }
  else if(pmode==PM_HIERARCH5_CUSHION){
#define distance(a,b) sqrt( powf(a,2.0) + powf(b,2.0))

#define d(x1,y1,x2,y2)  distance(x2-x1,y2-y1)

    float d1=d(x0,y0, c->cx0, c->cy0);
    float d2=d(x0+xd,y0, c->cx0, c->cy0);
    float d3=d(x0,y0+yd, c->cx0, c->cy0);
    float d4=d(x0+xd,y0+yd, c->cx0, c->cy0);
#if 0
    float d=MAX(d1,d2);
    d=MAX(d,d3);
    d=MAX(d,d4);
#endif
    float d=MIN(d1,d2);
    d=MIN(d,d3);
    d=MIN(d,d4);

    printf("%f,%f,%f,%f -> %f\n",d1,d2,d3,d4,d);
    //    float maxdist=d;
    float maxdist=sqrt( powf(xd/1.5,2.0) + powf(yd/1.5,2.0) );

    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){

	float dist= sqrt( powf(c->ncxd - w,2.0) + powf(c->ncyd - h,2.0) );


	//float i=dist/maxdist;
	float i=1.0-(dist/maxdist);
	//	float i=maxdist/dist;
	if(isnan(i)){
	  i=0.0;
	}
	i=MIN(1.0,i);
	i=MAX(0.0,i);
	//printf("i=%f\n",i);
	QColor newcol=QColor(check_int((int)(basecolor.red()*i)),
			     check_int((int)(basecolor.green()*i)),
			     check_int((int)(basecolor.blue()*i)));
	mypen.setColor(newcol);
	mypen.setWidth( 1);
	painter->setPen( mypen );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawPoint(x0+w,y0+h);

      }
    }
	mypen.setColor(basecolor);
	mypen.setWidth( 1);
	painter->setPen( mypen );
#if 0
    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);
#endif

  }
  else if(pmode==PM_HIERARCH_TEST_CUSHION){

    painter->fillRect(c->cx0,c->cy0,5,5,basecolor);
    painter->drawLine(x0,y0, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0, c->cx0, c->cy0);
    painter->drawLine(x0,y0+yd, c->cx0, c->cy0);
    painter->drawLine(x0+xd,y0+yd, c->cx0, c->cy0);

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
  else if(pmode==PM_WAVE2_CUSHION){
    //#define wcfunc(x)  (sin(((x+1.0)/2.0)*3.14))
    //#define wcfunc(x)  (((sin(((x+1.0)/2.0)*3.14))/2.0)+0.3)
#define wcfunc(x)  (((sin(((x+1.0)/2.0)*3.14))/2.0)+0.3)
#define wc2func(x)  (wcfunc(x)+(wcfunc(x*3.14)/3.0))

    for(int w=0;w<xd;w++){
      for(int h=0;h<yd;h++){
	float div_x=((float)w)/((float)xd); // range 0...1
	float div_y=((float)h)/((float)yd); // range 0...1

	float step1_x=(div_x-0.5)*2.0; // range -1...1
	float step1_y=(div_y-0.5)*2.0; // range -1...1

	float res_x=wc2func(step1_x); // range 0...1...0
	float res_y=wc2func(step1_y);

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

    float globmax=0.0;

    printf("x,y,xd,yd: %d,%d,%d,%d\n",x0,y0,xd,yd);
    printf("r[dy][0],r[dy][1],r[dx][0],r[dx][1]: %f,%f,%f,%f\n",c->r[VERTIKAL][0],c->r[VERTIKAL][1],c->r[HORIZONTAL][0],c->r[HORIZONTAL][1]);

    for(float iy=c->r[VERTIKAL][0] + 0.5 ; iy<=( c->r[VERTIKAL][1] - 0.5 );iy++){
      for(float ix=c->r[HORIZONTAL][0] + 0.5 ; ix<=( c->r[HORIZONTAL][1] - 0.5 );ix++){

    //    for(float iy=y0;iy<=y0+yd;iy++){
    //     for(float ix=x0;ix<=x0+xd;ix++){
	float nx=-(2*c->s[HORIZONTAL][2]*(ix+0.5) + c->s[HORIZONTAL][1] );
	float ny=-(2*c->s[VERTIKAL][2]*(iy+0.5) + c->s[VERTIKAL][1] );
	float cosa=(nx*Lx + ny*Ly + Lz )/sqrt(nx*nx + ny*ny +1.0);

	if(isnan(cosa)){
	  cosa=0.0;
	}
	float i=Ia+MAX(0,Is*cosa); // range?
	
	globmax=MAX(globmax,i);

	i=i/256.0;
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
    printf("globmax: %f\n",globmax);
      
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
