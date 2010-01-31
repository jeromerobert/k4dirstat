/*
 * k4dirstatview.h
 *
 * Copyright (C) 2007 %{AUTHOR} <%{EMAIL}>
 */
#ifndef K4DIRSTATVIEW_H
#define K4DIRSTATVIEW_H

#include <QtGui/QWidget>

#include "ui_k4dirstatview_base.h"

class QPainter;
class KUrl;

/**
 * This is the main view class for k4dirstat.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */

class k4dirstatView : public QWidget, public Ui::k4dirstatview_base
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    k4dirstatView(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~k4dirstatView();

private:
    Ui::k4dirstatview_base ui_k4dirstatview_base;

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

private slots:
    void switchColors();
    void settingsChanged();
};

#endif // k4dirstatVIEW_H
