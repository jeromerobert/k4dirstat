/*
 *   File name:	qtreemap.h
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: qtreemap.h,v 1.5 2001/07/05 23:15:37 alexannika Exp $
 *
 */

#ifndef QTreeMap_h
#define QTreeMap_h


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <qqueue.h>
#include <kfileitem.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qmainwindow.h>
//#include "kdirtree.h"
#include <qpen.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qbutton.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qscrollview.h>


#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif

// Open a new name space since KDE's name space is pretty much cluttered
// already - all names that would even remotely match are already used up,
// yet the resprective classes don't quite fit the purposes required here.

class Object;

namespace KDirStat
{

  // paint modes

#define PM_FLAT 0
#define PM_SIMPLE_CUSHION 1
#define PM_PYRAMID 2
#define PM_BITMAP 3
#define PM_PYRAMID_NICE_BUG 4
#define PM_OUTLINE 5
#define PM_CUSHION 6
#define PM_SQUARE_CUSHION 7
#define PM_CONE_CUSHION 8
#define PM_WAVE_CUSHION 9
#define PM_HIERARCH_CUSHION 10
#define PM_WAVE2_CUSHION 11
#define PM_HIERARCH2_CUSHION 12
#define PM_HIERARCH3_CUSHION 13
#define PM_HIERARCH4_CUSHION 14
#define PM_HIERARCH5_CUSHION 15
#define PM_HIERARCH_TEST_CUSHION 16

  // draw modes

#define DM_DIRS 0
#define DM_FILES 1
#define DM_BOTH 2

  // color scheme

#define CS_CYCLIC 0
#define CS_REGEXP 1
#define CS_MONO 2

  // color scheme type

#define CST_REGEXP 0
#define CST_PERMISSION 1
#define CST_USER 2

  // find modes

#define FIND_NOTHING 0
#define FIND_FILE 1
#define FIND_FIRSTDIR 2

  // directions

#define HORIZONTAL 0
#define VERTIKAL 1

#define DX HORIZONTAL
#define DY VERTIKAL

  // helpers

#define MAX(x,y) ( (x>y) ? x : y)
#define MIN(x,y) ( (x>y) ? y : x)

  class QTreeMapArea;
  class QTreeMapOptions;
  class QTreeMapWindow;

  class QTMcolorScheme {
  public:
    QString schemeName;
    QColor  color;
    int  type;
    QString  regexp;
    int  permission_mask;
    QString user;
  };

  class QTreeMapOptions {
  public:
    QTreeMapOptions();
    //~KTreeMapOptions();
    
    int   color_scheme;
    int dont_draw_xyd;
    int dont_descend_xyd;
    int draw_mode;
    int paintmode;
    bool start_direction;
    int step_width;
    int paint_size_x;
    int paint_size_y;
    int highlight_frame_width;
    QColor highlight_frame_col;
    bool draw_text;
    float hc_factor;
    int border_step;
    bool dynamic_shading;
    QColor mono_color;
    float sequoia_h;
    float sequoia_f;
  };

  class Cushion {
  public:
    Cushion(int xd,int yd,float sh,float sf);
    //    Cushion(Cushion *old);

    float    r[2][2];
    float    h;
    float    f;
    float    s[2][3];

    // hierarch. cushion

    int   cx0,cy0;
    float ncxd,ncyd;
  };

  class QTreeMapArea : public QWidget {
    Q_OBJECT

  public:
    //KTreeMap(QWidget *parent=0);
    QTreeMapArea(QWidget *parent=0);
    ~QTreeMapArea();

    void drawTreeMap(Object *dutree);
    void setTreeMap(Object *dutree);
    Object *findClickedMap(Object *dutree,int x,int y,int findmode);

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *event);

    void setOptions(QTreeMapOptions *opt);
    QTreeMapOptions *getOptions();
    void  resizeEvent( QResizeEvent *ev);
    void mousePressEvent(QMouseEvent *mouse);
    //  QString  tellUnit(int size);

    int check_int(int i);

  // pure virtual functions

    virtual QString fullName(Object *node) =0;
    virtual Object *firstChild(Object *node) =0;
    virtual int   totalSize(Object *node) =0;
    virtual bool isLeaf(Object *node)=0;
    virtual bool isNode(Object *node)=0;
    virtual bool isSameLevelChild(Object *node)=0;
    virtual Object *nextChild(Object *node)=0;
    virtual Object *sameLevelChild(Object *node)=0;
    virtual QString tellUnit(int size)=0;
    virtual Object *parentNode(Object *node)=0;
  

  private:

    void drawDuTree(Object *dutree, int x0,int y0,int xd0, int yd0, bool direction, int level,Cushion *cushion,int fx=-1,int fy=-1,int find_mode=FIND_NOTHING);

    void CTM(Object *tree,bool direction,Cushion *cushion);

    void paintEntry(int x0, int y0, int xd, int yd,QString entry_name,bool direction,int level,const QColor &basecolor,int pmode,Cushion *c);
  
    void initOptions();

  void appendRectlist(QString node_name,int x,int y,int xd,int yd);
  int getNextRotatingColorIndex();

  QColor& getBaseColor(QString name);

  QPainter *painter;
  QTreeMapOptions *options;
  QColor default_color;
  QWidget *widget;
  QPen mypen;
  QPixmap offscreen;
  //  QPixmap *offscreenptr;
  QPainter *win_painter;

  QColor rotate_colors[20];
  int color_index;

  Object *root_tree;

  QToolTip *tooltip;

  QVBox *info_box;
  QHBox *dir_box;

  Object *found_kfileinfo;
  QColor tooltipBgColor;
  QColor tooltipFgColor;
  QColor dirBaseColor;

  QPopupMenu *pop;

  Object *last_shaded;
  Object *next_shaded;
  // cushion rendering

  void cushion_AddRidge(float x1,float x2,float h,float& s1,float& s2);

#define R_MIN 0
#define R_MAX 1

  float cushion_rectangle[2][2];

#define D_1 0
#define D_2 1

  float cushion_surface[1][2];

  public slots:

  void directoryUp();
  void buttonUp();
  void deleteFile();
  void deleteFile(Object *convicted);
  void deleteFile(int id);
  void shellWindow(int id);
  void browserWindow(int id);
  void zoomIn();
  void zoomOut();

  signals:
      void highlighted(Object *high);
      void changedDirectory(Object *newdir);
  };

} // namespace


#endif // ifndef QTreeMap_h


// EOF
