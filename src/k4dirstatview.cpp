/*
 * k4dirstatview.cpp
 *
 * Copyright (C) 2008 %{AUTHOR} <%{EMAIL}>
 */
#include "k4dirstatview.h"
#include "settings.h"

#include <klocale.h>
#include <QtGui/QLabel>

k4dirstatView::k4dirstatView(QWidget *)
{
    ui_k4dirstatview_base.setupUi(this);
    settingsChanged();
    setAutoFillBackground(true);
}

k4dirstatView::~k4dirstatView()
{

}

void k4dirstatView::switchColors()
{
    // switch the foreground/background colors of the label
    QColor color = Settings::col_background();
    Settings::setCol_background( Settings::col_foreground() );
    Settings::setCol_foreground( color );

    settingsChanged();
}

void k4dirstatView::settingsChanged()
{
    QPalette pal;
    pal.setColor( QPalette::Window, Settings::col_background());
    pal.setColor( QPalette::WindowText, Settings::col_foreground());
    ui_k4dirstatview_base.kcfg_sillyLabel->setPalette( pal );

    // i18n : internationalization
    ui_k4dirstatview_base.kcfg_sillyLabel->setText( i18n("This project is %1 days old",Settings::val_time()) );
    emit signalChangeStatusbar( i18n("Settings changed") );
}

#include "k4dirstatview.moc"
