/*
 *   File name:	qtreemapwindow.h
 *   Summary:	Support classes for KDirStat
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-11
 *
 *   $Id: qtreemapwindow.h,v 1.5 2001/07/12 22:39:35 alexannika Exp $
 *
 */

#ifndef QTreeMapWindow_h
#define QTreeMapWindow_h


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
#include "kdirtree.h"
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
#include <qlineedit.h>

#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif

// Open a new name space since KDE's name space is pretty much cluttered
// already - all names that would even remotely match are already used up,
// yet the resprective classes don't quite fit the purposes required here.

class Object;

namespace KDirStat
{

  class QTreeMapArea;
  class QTreeMapOptions;
  class QTreeMapWindow;

  class QTreeMapWindow : public QMainWindow {
    Q_OBJECT

  public:
    //QTreeMapWindow(QWidget * parent = 0 );
    QTreeMapWindow( );

    QTreeMapArea *getArea();

    void makeRadioPopup(QPopupMenu *menu,const QString& title, const char *slot,const int param);

    void makeWidgets();

    // pure virtual methods

    virtual  QTreeMapArea *makeTreeMapWidget(QWidget *parent) =0 ;

    public slots:

      void findMatch();
    void setStatusBar(Object *found);
    void setDirectoryLabel(Object *newdir);
    void selectDrawmode(int id);
    void selectShading(int id);
    void selectBorderWidth(int id);
    void selectBorderStep(int id);
    void selectStartDirection(int id);
    void selectColorScheme(int id);
    void changeDrawText(int id);
    void changeDynamicShading(int id);
    void changeSquarifyTreemaps(int id);
    void changeShowInodeSpace(int id);
    void selectDontDrawOption(int id);
    void selectHFactor(int id);
    void selectSHFactor(int id);
    void selectSFFactor(int id);
    void redoOptions();

    //void paintEvent(QPaintEvent *event);


  private:

    QTreeMapOptions *options;

    // toolbar widgets
  QLabel *dir_name_label;
  QLabel *info_label;
  QPushButton *up_button;
  QLineEdit *find_entry;

  QTreeMapArea *graph_widget;

  QPushButton *test_button;

  QPopupMenu *popup;
  int popup_id;

  QMenuBar *menubar;
  QStatusBar *statusbar;
  QToolBar  *toolbar;
  QPopupMenu *menu_file;
  QPopupMenu *menu_options;
  QPopupMenu *menu_paint_mode;
  QPopupMenu *menu_draw_mode;
  QPopupMenu *menu_border_width;
  QPopupMenu *menu_border_step;
  QPopupMenu *menu_start_direction;
  QPopupMenu *menu_dont_draw;
  QPopupMenu *menu_hfactor;
  QPopupMenu *menu_shfactor;
  QPopupMenu *menu_sffactor;
  QPopupMenu *menu_colorscheme;

  QPushButton *zoom_out_button;
  QPushButton *zoom_in_button;

  QButtonGroup *radio_group;
  
  QScrollView *scrollview;
  QWidget *desktop;

  int menu_file_id,menu_options_id;

  };

} // namespace


#endif // ifndef QTreeMapWindow_h


// EOF
