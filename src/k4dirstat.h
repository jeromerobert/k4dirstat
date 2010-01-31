/*
 * k4dirstat.h
 *
 * Copyright (C) 2008 %{AUTHOR} <%{EMAIL}>
 */
#ifndef K4DIRSTAT_H
#define K4DIRSTAT_H


#include <kxmlguiwindow.h>

#include "ui_prefs_base.h"

class k4dirstatView;
class QPrinter;
class KToggleAction;
class KUrl;

/**
 * This class serves as the main window for k4dirstat.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class k4dirstat : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    k4dirstat();

    /**
     * Default Destructor
     */
    virtual ~k4dirstat();

private slots:
    void fileNew();
    void optionsPreferences();

private:
    void setupActions();

private:
    Ui::prefs_base ui_prefs_base ;
    k4dirstatView *m_view;

    QPrinter   *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _K4DIRSTAT_H_
