/*
 *   File name:	qxmltreemapviewer.cpp
 *   Summary:	stand-alone program to display a treemap
 *              that gets loaded from a xml-treemap file
 *   License:	GPL - See file COPYING for details.
 *
 *   Author:	Alexander Rawass <alexannika@users.sourceforge.net>
 *
 *   Updated:	2001-06-14
 *
 *   $Id: qxmltreemapviewer.cpp,v 1.1 2001/08/10 03:45:48 alexannika Exp $
 *
 */


#include <qapplication.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qfiledialog.h>
#include "qtreemap.h"
#include "qtreemapwindow.h"
#include "qxmltreemap.h"
#include "qxmltreemapwindow.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
  QString filename=QFileDialog::getOpenFileName("treemap.xml", "XML (*.xml)" );

  if(!filename.isEmpty()){

  QDomDocument doc(filename);

  QFile file(filename);
  if(!file.open(IO_ReadOnly)){
    return 1;
  }
  if(!doc.setContent(&file)){
    file.close();
    return 1;
  }
  file.close();

  QDomElement docElem = doc.documentElement();

  //  xmlwalker(docElem,0);

  
  //  return;

  QDomElement root_elem=doc.documentElement();
  QXmlTreeMapWindow *xml_treemap_window=new QXmlTreeMapWindow();
  xml_treemap_window->makeWidgets();
  //  xml_treemap_window->getArea()->setOptions(options);
  xml_treemap_window->getArea()->setTreeMap((Object *)(new QDomElement(root_elem)));


    a.setMainWidget( xml_treemap_window );
    //    hello.show();
  }
    return a.exec();
}
