/*
 *   File name:	kdirstatsettings.cpp
 *   Summary:	Settings dialog for KDirStat
 *   License:	GPL - See file COPYING for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-02-24
 *
 *   $Id: kdirstatsettings.cpp,v 1.2 2002/02/25 10:49:07 hundhammer Exp $
 *
 */


#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qslider.h>
#include <qvbox.h>

#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kdirtreeview.h"
#include "kdirstatapp.h"
#include "kdirstatsettings.h"


using namespace KDirStat;


KSettingsDialog::KSettingsDialog( KDirStatApp *mainWin )
    : KDialogBase( Tabbed,					// dialogFace
		   i18n( "Settings" ),				// caption
		   Ok | Apply | Default | Cancel | Help,	// buttonMask
		   Ok,						// defaultButton
		   0,						// parent
		   0,						// name
		   false )					// modal
    , _mainWin( mainWin )
{
    /**
     * This may seem like overkill, but I didn't find any other way to get
     * geometry management right with KDialogBase, yet maintain a modular and
     * object-oriented design:
     *
     * Each individual settings page is added with 'addVBoxPage()' to get some
     * initial geometry management. Only then can some generic widget be added
     * into this - and I WANT my settings pages to be generic widgets. I want
     * them to be self-sufficient - no monolithic mess of widget creation in my
     * code, intermixed with all kinds of layout objects.
     *
     * The ordinary KDialogBase::addPage() just creates a QFrame which is too
     * dumb for any kind of geometry management - it cannot even handle one
     * single child right. So, let's have KDialogBase create something more
     * intelligent: A QVBox (which is derived from QFrame anyway). This QVBox
     * gets only one child - the KSettingsPage. This KSettingsPage handles its
     * own layout.
     **/
    
    QWidget * page;
    
    page = addVBoxPage( i18n( "&Cleanups" ) );
    _cleanupsPageIndex = pageIndex( page );
    new KCleanupPage( this, page, _mainWin );
    
    page = addVBoxPage( i18n( "&Tree Colors" ) );
    _treeColorsPageIndex = pageIndex( page );
    new KTreeColorsPage( this, page, _mainWin );

    // resize( sizeHint() );
}


KSettingsDialog::~KSettingsDialog()
{
    // NOP
}


void
KSettingsDialog::show()
{
    emit aboutToShow();
    KDialogBase::show();
}


void
KSettingsDialog::slotDefault()
{
    if ( KMessageBox::warningContinueCancel( this,
					     i18n( "Really revert all settings to their default values?\n"
						   "You will lose all changes you ever made!" ),
					     i18n( "Please confirm" ),			// caption
					     i18n( "&Really revert to defaults" )	// continueButton
					     ) == KMessageBox::Continue )
    {
	emit defaultClicked();
	emit applyClicked();
    }
}


void
KSettingsDialog::slotHelp()
{
    QString helpTopic = "";
    
    if      ( activePageIndex() == _cleanupsPageIndex )		helpTopic = "configuring_cleanups";
    else if ( activePageIndex() == _treeColorsPageIndex )	helpTopic = "tree_colors";

    // kdDebug() << "Help topic: " << helpTopic << endl;
    kapp->invokeHelp( helpTopic );
}


/*--------------------------------------------------------------------------*/


KSettingsPage::KSettingsPage( KSettingsDialog *	dialog,
			      QWidget *		parent )
    : QWidget( parent )
{
    connect( dialog,	SIGNAL( aboutToShow	( void ) ),
	     this,	SLOT  ( setup      	( void ) ) );

    connect( dialog,	SIGNAL( okClicked	( void ) ),
	     this,	SLOT  ( apply       	( void ) ) );
    
    connect( dialog,	SIGNAL( applyClicked	( void ) ),
	     this,	SLOT  ( apply       	( void ) ) );
    
    connect( dialog,	SIGNAL( defaultClicked  ( void ) ),
	     this,	SLOT  ( revertToDefaults( void ) ) );
}


KSettingsPage::~KSettingsPage()
{
    // NOP
}

	
/*--------------------------------------------------------------------------*/


KTreeColorsPage::KTreeColorsPage( KSettingsDialog *	dialog,
				  QWidget *		parent,
				  KDirStatApp *		mainWin )
    : KSettingsPage( dialog, parent )
    , _mainWin( mainWin )
    , _treeView( mainWin->treeView() )
    , _maxButtons( KDirStatSettingsMaxColorButton )
{
    // Outer layout box

    QHBoxLayout * outerBox = new QHBoxLayout( this,
					      0,	// border
					      dialog->spacingHint() );

   
    // Inner layout box with a column of color buttons
   
    QGridLayout *grid = new QGridLayout( _maxButtons,		// rows
					 _maxButtons + 1,	// cols
					 dialog->spacingHint() );
    outerBox->addLayout( grid, 1 );
    grid->setColStretch( 0, 0 ); 	// label column - dont' stretch
   
    for ( int i=1; i < _maxButtons; i++ )
    {
	grid->setColStretch( i, 1 );	// all other columns stretch as you like
    }
   
    for ( int i=0; i < _maxButtons; i++ )
    {
	QString labelText;
      
	labelText.sprintf( i18n( "Tree level %d" ), i+1 );
	_colorLabel[i] = new QLabel( labelText, this );
	grid->addWidget( _colorLabel [i], i, 0 );
      
	_colorButton[i] = new KColorButton( this );
	_colorButton[i]->setMinimumSize( QSize( 80, 10 ) );
	grid->addMultiCellWidget( _colorButton [i], i, i, i+1, _maxButtons );
	grid->setRowStretch( i, 1 );
    }

   
    // Vertical slider
   
    _slider = new QSlider( 1,			// minValue
			   _maxButtons,		// maxValue
			   1,			// pageStep
			   1,			// value
			   QSlider::Vertical,
			   this );
    outerBox->addWidget( _slider, 0 );
    outerBox->activate();

    connect( _slider,	SIGNAL( valueChanged( int ) ),
	     this,	SLOT  ( enableColors( int ) ) );
}


KTreeColorsPage::~KTreeColorsPage()
{
    // NOP
}


void
KTreeColorsPage::apply()
{
    _treeView->setUsedFillColors( _slider->value() );
      
    for ( int i=0; i < _maxButtons; i++ )
    {
	_treeView->setFillColor( i, _colorButton [i]->color() );
    }

    _treeView->triggerUpdate();
}


void
KTreeColorsPage::revertToDefaults()
{
    _treeView->setDefaultFillColors();
    setup();
}


void
KTreeColorsPage::setup()
{
    for ( int i=0; i < _maxButtons; i++ )
    {
	_colorButton [i]->setColor( _treeView->rawFillColor(i) );
    }

    _slider->setValue( _treeView->usedFillColors() );
    enableColors( _treeView->usedFillColors() );
}


void
KTreeColorsPage::enableColors( int maxColors )
{
    for ( int i=0; i < _maxButtons; i++ )
    {
	_colorButton [i]->setEnabled( i < maxColors );
	_colorLabel  [i]->setEnabled( i < maxColors );
    }
}


/*--------------------------------------------------------------------------*/



KCleanupPage::KCleanupPage( KSettingsDialog *	dialog,
			    QWidget *		parent,
			    KDirStatApp *	mainWin )
    : KSettingsPage( dialog, parent )
    , _mainWin( mainWin )
    , _currentCleanup( 0 )
{
    // Copy the main window's cleanup collection. 
    
    _workCleanupCollection = *mainWin->cleanupCollection();

    // Create layout and widgets.

    QHBoxLayout * layout = new QHBoxLayout( this,
					    5,		// border
					    5 );	// spacing
    _listBox	= new KCleanupListBox( this );
    _props	= new KCleanupPropertiesPage( this, mainWin );

   
    // Connect list box signals to reflect changes in the list
    // selection in the cleanup properties page - whenever the user
    // clicks on a different cleanup in the list, the properties page
    // will display that cleanup's values.
   
    connect( _listBox, SIGNAL( selectCleanup( KCleanup * ) ),
	     this,     SLOT  ( changeCleanup( KCleanup * ) ) );

    
    // Fill list box so it can determine a reasonable startup geometry - that
    // doesn't work if it happens only later.
    
    setup();
    
    // Now that _listBox will (hopefully) have determined a reasonable
    // default geometry, add the widgets to the layout.

    layout->addWidget( _listBox, 0 );
    layout->addWidget( _props  , 1 );
    layout->activate();

#if 0
    setMinimumSize( sizeHint() );
#endif
}


KCleanupPage::~KCleanupPage()
{
    // NOP
}


void
KCleanupPage::changeCleanup( KCleanup * newCleanup )
{
    if ( _currentCleanup && newCleanup != _currentCleanup )
    {
	storeProps( _currentCleanup );
    }

    _currentCleanup = newCleanup;
    _props->setFields( _currentCleanup );
}


void
KCleanupPage::apply()
{
    exportCleanups();
}


void
KCleanupPage::revertToDefaults()
{
    _mainWin->revertCleanupsToDefaults();
    setup();
}


void
KCleanupPage::setup()
{
    importCleanups();

    // Fill the list box.

    _listBox->clear();
    KCleanupList cleanupList = _workCleanupCollection.cleanupList();
    KCleanupListIterator it( cleanupList );

    while ( *it )
    {
	_listBox->insert( *it );
	++it;
    }


    // (Re-) Initialize list box.

    // _listBox->resize( _listBox->sizeHint() );
    _listBox->setSelected( 0, true );
}


void
KCleanupPage::importCleanups()
{
    // Copy the main window's cleanup collecton to _workCleanupCollection.
    
    _workCleanupCollection = * _mainWin->cleanupCollection();


    // Pointers to the old collection contents are now invalid!
    
    _currentCleanup = 0;
}


void
KCleanupPage::exportCleanups()
{
    // Retrieve any pending changes from the properties page and store
    // them in the current cleanup.
    
    storeProps( _currentCleanup );

    
    // Copy the _workCleanupCollection to the main window's cleanup
    // collection.
    
    * _mainWin->cleanupCollection() = _workCleanupCollection;
}


void
KCleanupPage::storeProps( KCleanup * cleanup )
{
    if ( cleanup )
    {
	// Retrieve the current fields contents and store them in the current
	// cleanup.
	
	*cleanup = _props->fields();

	// Update the list box accordingly - the cleanup's title may have
	// changed, too!
	
	_listBox->updateTitle( cleanup );
    }
}

/*--------------------------------------------------------------------------*/


KCleanupListBox::KCleanupListBox( QWidget *parent )
   : QListBox( parent )
{
    _selection = 0;
    
    connect( this,
	     SIGNAL( selectionChanged( QListBoxItem *) ),
	     SLOT  ( selectCleanup   ( QListBoxItem *) ) );
}


QSize
KCleanupListBox::sizeHint() const
{
    // FIXME: Is this still needed with Qt 2.x?
    
    if ( count() < 1 )
    {
	// As long as the list is empty, sizeHint() would default to
	// (0,0) which is ALWAYS just a pain in the ass. We'd rather
	// have an absolutely random value than this.
	return QSize( 100, 100 );
    }
    else
    {
	// Calculate the list contents and take 3D frames (2*2 pixels)
	// into account.
	return QSize ( maxItemWidth() + 5,
		       count() * itemHeight( 0 ) + 4 );
    }
}


void
KCleanupListBox::insert( KCleanup * cleanup )
{
    // Create a new listbox item - this will insert itself (!) automatically.
    // It took me half an afternoon to figure _this_ out. Not too intuitive
    // when there is an insertItem() method, too, eh?

    new KCleanupListBoxItem( this, cleanup );
}


void
KCleanupListBox::selectCleanup( QListBoxItem * listBoxItem )
{
    KCleanupListBoxItem * item = (KCleanupListBoxItem *) listBoxItem;

    _selection = item->cleanup();
    emit selectCleanup( _selection );
}


void
KCleanupListBox::updateTitle( KCleanup * cleanup )
{
    KCleanupListBoxItem * item = (KCleanupListBoxItem *) firstItem();

    while ( item )
    {
	if ( ! cleanup || item->cleanup() == cleanup )
	    item->updateTitle();
	
	item = (KCleanupListBoxItem *) item->next();
    }
}


/*--------------------------------------------------------------------------*/


KCleanupListBoxItem::KCleanupListBoxItem( KCleanupListBox *	listBox,
					  KCleanup *		cleanup )
    : QListBoxText( listBox )
    , _cleanup( cleanup )
{
    CHECK_PTR( cleanup );
    setText( cleanup->cleanTitle() );
}


void
KCleanupListBoxItem::updateTitle()
{
    setText( _cleanup->cleanTitle() );
}


/*--------------------------------------------------------------------------*/


KCleanupPropertiesPage::KCleanupPropertiesPage( QWidget *	parent,
						KDirStatApp *	mainWin )
   : QWidget( parent )
   , _mainWin( mainWin )
{
    QVBoxLayout	*outerBox = new QVBoxLayout( this,
					     0,		// border
					     0 );	// spacing

    // The topmost check box: "Enabled".
   
    _enabled = new QCheckBox( i18n( "&Enabled" ), this );
    outerBox->addWidget( _enabled, 0 );
    outerBox->addSpacing( 7 );
    outerBox->addStretch();

    connect( _enabled, 	SIGNAL( toggled     ( bool ) ),
	     this,	SLOT  ( enableFields( bool ) ) );

   
    // All other widgets of this page are grouped together in a
    // separate subwidget so they can all be enabled / disabled
    // together.
    _fields  = new QWidget( this );
    outerBox->addWidget( _fields, 1 );

    QVBoxLayout *fieldsBox = new QVBoxLayout( _fields );

   
    // Grid layout for the edit fields, their labels, some
    // explanatory text and the "recurse?" check box.
   
    QGridLayout *grid = new QGridLayout( 6,	// rows
					 2,	// cols
					 4 );	// spacing
    fieldsBox->addLayout( grid, 0 );
    fieldsBox->addStretch();
    fieldsBox->addSpacing( 5 );

    grid->setColStretch( 0, 0 ); // column for field labels - dont' stretch
    grid->setColStretch( 1, 1 ); // column for edit fields - stretch as you like


    // Edit fields for cleanup action title and command line.
   
    QLabel *label;
    _title	= new QLineEdit( _fields );					grid->addWidget( _title,   0, 1 );
    _command	= new QLineEdit( _fields );					grid->addWidget( _command, 1, 1 );
    label	= new QLabel( _title,   i18n( "&Title:"		), _fields );	grid->addWidget( label,    0, 0 );
    label	= new QLabel( _command, i18n( "&Command line:"	), _fields );	grid->addWidget( label,    1, 0 );

    label = new QLabel( i18n( "%p full path" ), _fields );
    grid->addWidget( label, 2, 1 );

    label = new QLabel( i18n( "%n file / directory name without path"), _fields );
    grid->addWidget( label, 3, 1 );


    // "Recurse into subdirs" check box
   
    _recurse = new QCheckBox( i18n( "&Recurse into subdirectories" ), _fields );
    grid->addWidget( _recurse, 4, 1 );

    // "Ask for confirmation" check box

    _askForConfirmation = new QCheckBox( i18n( "&Ask for confirmation" ), _fields );
    grid->addWidget( _askForConfirmation, 5, 1 );

    
    // The "Works for..." check boxes, grouped together in a button group.
   
    QButtonGroup *worksFor = new QButtonGroup( i18n( "Works for..." ), _fields );
    QVBoxLayout *worksForBox = new QVBoxLayout( worksFor, 15, 2 );
    fieldsBox->addWidget( worksFor, 0 );
    fieldsBox->addSpacing( 5 );
    fieldsBox->addStretch();

    _worksForDir	= new QCheckBox( i18n( "&Directories"		), worksFor );
    _worksForFile	= new QCheckBox( i18n( "&Files"			), worksFor );
    _worksForDotEntry	= new QCheckBox( i18n( "<Files> p&seudo entries"), worksFor );

    worksForBox->addWidget( _worksForDir	, 1 );
    worksForBox->addWidget( _worksForFile	, 1 );
    worksForBox->addWidget( _worksForDotEntry	, 1 );

    worksForBox->addSpacing( 5 );
    _worksForProtocols = new QComboBox( false, worksFor );
    worksForBox->addWidget( _worksForProtocols, 1 );

    _worksForProtocols->insertItem( i18n( "On local machine only ('file:/' protocol)" ) );
    _worksForProtocols->insertItem( i18n( "Network transparent (ftp, smb, tar, ...)" ) );


    // Grid layout for combo boxes at the bottom
    
    grid = new QGridLayout( 1,		// rows
			    2,		// cols
			    4 );	// spacing
    
    fieldsBox->addLayout( grid, 0 );
    fieldsBox->addSpacing( 5 );
    fieldsBox->addStretch();
    int row = 0;

    
    // The "Refresh policy" combo box
   
    _refreshPolicy = new QComboBox( false, _fields );
    grid->addWidget( _refreshPolicy, row, 1 );
    
    label = new QLabel( _refreshPolicy, i18n( "Refresh &Policy:" ), _fields );
    grid->addWidget( label, row++, 0 );


    // Caution: The order of those entries must match the order of
    // 'enum RefreshPolicy' in 'kcleanup.h'!
    //
    // I don't like this one bit. The ComboBox should provide something better
    // than mere numeric IDs. One of these days I'm going to rewrite this
    // thing!
    
    _refreshPolicy->insertItem( i18n( "No refresh" 			) );
    _refreshPolicy->insertItem( i18n( "Refresh this entry"		) );
    _refreshPolicy->insertItem( i18n( "Refresh this entry's parent"	) );
    _refreshPolicy->insertItem( i18n( "Assume entry has been deleted"	) );

    
    outerBox->activate();
    setMinimumSize( sizeHint() );
}


void
KCleanupPropertiesPage::enableFields( bool active )
{
    _fields->setEnabled( active );
}


void
KCleanupPropertiesPage::setFields( const KCleanup * cleanup )
{
    _id = cleanup->id();
    _enabled->setChecked		( cleanup->enabled()		);
    _title->setText			( cleanup->title()		);
    _command->setText			( cleanup->command()		);
    _recurse->setChecked		( cleanup->recurse()		);
    _askForConfirmation->setChecked	( cleanup->askForConfirmation()	);
    _worksForDir->setChecked		( cleanup->worksForDir()	);
    _worksForFile->setChecked		( cleanup->worksForFile()	);
    _worksForDotEntry->setChecked	( cleanup->worksForDotEntry()	);
    _worksForProtocols->setCurrentItem	( cleanup->worksLocalOnly() ? 0 : 1 );
    _refreshPolicy->setCurrentItem	( cleanup->refreshPolicy() 	);

    enableFields( cleanup->enabled() );
}


KCleanup
KCleanupPropertiesPage::fields() const
{
    KCleanup cleanup( _id );
   
    cleanup.setEnabled			( _enabled->isChecked()		   );
    cleanup.setTitle			( _title->text()		   );
    cleanup.setCommand			( _command->text()		   );
    cleanup.setRecurse			( _recurse->isChecked()		   );
    cleanup.setAskForConfirmation	( _askForConfirmation->isChecked() );
    cleanup.setWorksForDir		( _worksForDir->isChecked()	   );
    cleanup.setWorksForFile		( _worksForFile->isChecked()	   );
    cleanup.setWorksLocalOnly		( _worksForProtocols->currentItem() == 0 ? true : false );
    cleanup.setWorksForDotEntry		( _worksForDotEntry->isChecked()   );
    cleanup.setRefreshPolicy		( (KCleanup::RefreshPolicy) _refreshPolicy->currentItem() );

    return cleanup;
}





// EOF
