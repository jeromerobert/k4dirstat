/****************************************************************************
** KDirStat::KTreeMap meta object code from reading C++ file 'ktreemap.h'
**
** Created: Tue Jun 19 18:47:03 2001
**      by: The Qt MOC ($Id: ktreemap.moc.cpp,v 1.1 2001/06/29 16:37:50 hundhammer Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "ktreemap.h"
#include <qmetaobject.h>
#include <qapplication.h>



const char *KDirStat::KTreeMap::className() const
{
    return "KDirStat::KTreeMap";
}

QMetaObject *KDirStat::KTreeMap::metaObj = 0;

void KDirStat::KTreeMap::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(QMainWindow::className(), "QMainWindow") != 0 )
	badSuperclassWarning("KDirStat::KTreeMap","QMainWindow");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString KDirStat::KTreeMap::tr(const char* s)
{
    return qApp->translate( "KDirStat::KTreeMap", s, 0 );
}

QString KDirStat::KTreeMap::tr(const char* s, const char * c)
{
    return qApp->translate( "KDirStat::KTreeMap", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* KDirStat::KTreeMap::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QMainWindow::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (KDirStat::KTreeMap::*m1_t0)();
    typedef void (QObject::*om1_t0)();
    m1_t0 v1_0 = &KDirStat::KTreeMap::buttonUp;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    QMetaData *slot_tbl = QMetaObject::new_metadata(1);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(1);
    slot_tbl[0].name = "buttonUp()";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl_access[0] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"KDirStat::KTreeMap", "QMainWindow",
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
