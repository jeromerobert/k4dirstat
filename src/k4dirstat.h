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

class Q3PopupMenu;
class QSplitter;
class KActivityTracker;
class KFeedbackDialog;
class KPacMan;
class KAction;
class KRecentFilesAction;
class KToggleAction;

namespace KDirStat
{
    class KCleanupCollection;
    class KDirTreeView;
    class KDirTreeViewItem;
    class KFileInfo;
    class KSettingsDialog;
    class KTreemapView;
    class KTreemapTile;
}

using namespace KDirStat;

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

    /**
     * Open an URL specified by command line argument.
     **/
    void openURL( const KUrl & url );

    /**
     * Return the main window's @ref KDirTreeView.
     **/
    KDirTreeView * treeView() const { return _treeView; }

    /**
     * Returns the main window's @ref KTreemapView or 0 if there is none.
     *
     * Caution: Do not try to cache this value. The treemap view is destroyed
     * and re-created frequently!
     **/
    KTreemapView * treemapView() const { return _treemapView; }

public slots:
    /**
     * Open a directory tree.
     **/
    void fileAskOpenDir();

    /**
     * Open a (possibly remote) directory tree.
     **/
    void fileAskOpenUrl();

    /**
     * Refresh the entire directory tree, i.e. re-read everything from disk.
     **/
    void refreshAll();

    /**
     * Refresh the selected subtree, i.e. re-read it from disk.
     **/
    void refreshSelected();

    /**
     * Refresh the entire directory tree, i.e. re-read everything from disk.
     **/
    void stopReading();

    /**
     * Open a directory tree from the "recent" menu.
     **/
    void fileOpenRecent( const KUrl& url );

    /**
     * asks for saving if the file is modified, then closes the current file
     * and window
     **/
    void fileCloseDir();

    /**
     * put the marked text/object into the clipboard
     **/
    void editCopy();

    /**
     * Notification that the view's selection has changed.
     * Enable/disable user actions as appropriate.
     **/
    void selectionChanged( KFileInfo *selection );

    /**
     * Ask user what application to open a file or directory with
     **/
    void cleanupOpenWith();

    /**
     * Toggle treemap view
     **/
    void toggleTreemapView();

    /**
     * Zoom in the treemap at the currently selected tile.
     **/
    void treemapZoomIn();

    /**
     * Zoom out the treemap after zooming in.
     **/
    void treemapZoomOut();

    /**
     * Select the parent of the currently selected treemap tile.
     **/
    void treemapSelectParent();

    /**
     * Rebuild the treemap.
     **/
    void treemapRebuild();

    /**
     * Invoke online help about treemaps.
     **/
    void treemapHelp();

    /**
     * Open settings dialog
     **/
    void preferences();

    /**
     * Changes the statusbar contents for the standard label permanently, used
     * to indicate current actions.
     *
     * @param text the text that is displayed in the statusbar
     **/
    void statusMsg( const QString &text );

    /**
     * Opens a context menu for tree view items.
     **/
    void contextMenu( KDirTreeViewItem * item, const QPoint &pos );

    /**
     * Opens a context menu for treemap tiles.
     **/
    void contextMenu( KTreemapTile * tile, const QPoint &pos );

    /**
     * Create a treemap view. This makes only sense after a directory tree is
     * completely read.
     **/
    void createTreemapView();

    /**
     * Create a treemap view after all events are processed.
     **/
    void createTreemapViewDelayed();

    /**
     * Delete an existing treemap view if there is one.
     **/
    void deleteTreemapView();

    /**
     * Sends a user feedback mail.
     **/
    //void sendFeedbackMail();

    /**
     * Read configuration for the main window.
     **/
    void readMainWinConfig();

    /**
     * Save the main window's configuration.
     **/
    void saveMainWinConfig();

    /**
     * Revert all cleanups to default values.
     **/
    void revertCleanupsToDefaults();

    /**
     * For the settings dialog only: Return the internal cleanup collection.
     **/
    KCleanupCollection * cleanupCollection() { return _cleanupCollection; }

    /**
     * Initialize @ref KPacMan animation in the tool bar.
     **/
    void initPacMan( bool enablePacMan = true );

    /**
     * Returns true if the pacman animation in the tool bar is enabled, false
     * otherwise.
     **/
    bool pacManEnabled() const { return _pacMan != 0; }

    /**
     * Ask user if he wouldn't like to rate this program.
     **/
    //void askForFeedback();

    /**
     * Notification that a feedback mail has been sent, thus don't remind
     * the user any more.
     **/
    //void feedbackMailSent();

    /**
     * Update enabled/disabled state of the user actions.
     **/
    void updateActions();

    /**
     * Open a file selection box to save the current directory tree to a
     * kdirstat cache file
     **/
    void askWriteCache();

    /**
     * Open a file selection box to read a directory tree from a kdirstat cache
     * file
     **/
    void askReadCache();

signals:

    /**
     * Emitted when the configuration is to be read - other than at program
     * startup / object creation where each object is responsible for reading
     * its configuraton at an appropriate time.
     **/
    void readConfig();

    /**
     * Emitted when the configuration is to be saved.
     **/
    void saveConfig();

//private slots:
//    void optionsPreferences();

protected:

    /**
     * Initialize @ref KCleanup actions.
     **/
    void initCleanups();

    // Widgets

    QSplitter *			_splitter;
    KDirTreeView *		    _treeView;
    KTreemapView *		    _treemapView;
    KPacMan *			_pacMan;
    QWidget *			_pacManDelimiter;
    Q3PopupMenu *		_treeViewContextMenu;
    Q3PopupMenu *		_treemapContextMenu;
    KDirStat::KSettingsDialog *	_settingsDialog;
    KFeedbackDialog *		_feedbackDialog;
    KActivityTracker *		_activityTracker;

    KAction * 			_fileAskOpenDir;
    KAction * 			_fileAskOpenUrl;
    KRecentFilesAction *	_fileOpenRecent;
    KAction * 			_fileCloseDir;
    KAction * 			_fileRefreshAll;
    KAction *			_fileRefreshSelected;
    KAction *			_fileReadExcludedDir;
    KAction *			_fileContinueReadingAtMountPoint;
    KAction *			_fileStopReading;
    KAction *			_fileAskWriteCache;
    KAction *			_fileAskReadCache;
    KAction * 			_fileQuit;
    KAction * 			_editCopy;
    KAction * 			_cleanupOpenWith;
    KAction *	 		_treemapZoomIn;
    KAction *			_treemapZoomOut;
    KAction *			_treemapSelectParent;
    KAction * 			_treemapRebuild;

    KAction *			_reportMailToOwner;
    KAction *			_helpSendFeedbackMail;
    KToggleAction * 		_showTreemapView;

    KCleanupCollection *	_cleanupCollection;

    int				_treemapViewHeight;

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
