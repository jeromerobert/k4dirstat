/*
 *   File name: kdirstatsettings.cpp
 *   Summary:	Settings dialog for KDirStat
 *   License:	GPL - See file COPYING for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 *
 *   Updated:	2010-02-20
 */


#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qinputdialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>
#include <kvbox.h>

#include "kdirtreeview.h"
#include "ktreemapview.h"
#include "kdirstatsettings.h"
#include "kexcluderules.h"
#include <QGroupBox>
#include <QMenu>
#include <KDialog>
#include <KGlobal>
#include <KHelpClient>

using namespace KDirStat;


KSettingsDialog::KSettingsDialog( k4dirstat *mainWin )
    : KPageDialog(mainWin)
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

    setFaceType(Tabbed);
    setModal(false);
    setWindowTitle(i18n( "Settings"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply |
                       QDialogButtonBox::RestoreDefaults |
                       QDialogButtonBox::Cancel | QDialogButtonBox::Help);

    KVBox * page = new KVBox();
    KPageWidgetItem *item;

    item = addPage(page, i18n( "&Actions" ) );
    _cleanupsPageIndex = item;
    _pages.append(new KCleanupPage(page, _mainWin ));

    page = new KVBox();
    _treeColorsPageIndex = addPage(page, i18n( "&Tree Colors" ));
    _pages.append(new KTreeColorsPage(page, _mainWin ));

    page = new KVBox();
    _treemapPageIndex = addPage(page, i18n( "Tree&map" ) );
    _pages.append(new KTreemapPage(page, _mainWin ));

    page = new KVBox();
    _generalSettingsPageIndex = addPage(page, i18n( "&General" ) );
    _pages.append(new KGeneralSettingsPage(page, _mainWin ));

    connect(this, SIGNAL(aboutToShow()), this, SLOT(setup()));
    connect(button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(apply()));
    connect(button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
    connect(button(QDialogButtonBox::RestoreDefaults),
            SIGNAL(clicked()), this, SLOT(revertToDefaults()));
}

void KSettingsDialog::apply() {
    foreach(KSettingsPage * p, _pages)
        p->apply();
}

void KSettingsDialog::revertToDefaults() {
    foreach(KSettingsPage * p, _pages)
        p->revertToDefaults();
}

void KSettingsDialog::setup() {
    foreach(KSettingsPage * p, _pages)
        p->setup();
}

KSettingsDialog::~KSettingsDialog()
{
    // NOP
}


void
KSettingsDialog::show()
{
    emit aboutToShow();
    KPageDialog::show();
}


void
KSettingsDialog::slotDefault()
{
    if ( KMessageBox::warningContinueCancel( this,
					     i18n( "Really revert all settings to their default values?\n"
						   "You will lose all changes you ever made!" ),
					     i18n( "Please Confirm" ),			// caption
                                             KGuiItem(i18n( "&Really Revert to Defaults" ))	// continueButton
					     ) == KMessageBox::Continue )
    {
        revertToDefaults();
        apply();
    }
}


void
KSettingsDialog::slotHelp()
{
    QString helpTopic = "";

    if	    ( currentPage() == _cleanupsPageIndex	)	helpTopic = "configuring_cleanups";
    else if ( currentPage() == _treeColorsPageIndex )	helpTopic = "tree_colors";
    else if ( currentPage() == _treemapPageIndex	)	helpTopic = "treemap_settings";
    else if ( currentPage() == _generalSettingsPageIndex)	helpTopic = "general_settings";

    // qDebug() << "Help topic: " << helpTopic << endl;
    KHelpClient::invokeHelp(helpTopic);
}


/*--------------------------------------------------------------------------*/

KSettingsPage::~KSettingsPage()
{
    // NOP
}


/*--------------------------------------------------------------------------*/


KTreeColorsPage::KTreeColorsPage( QWidget *		parent,
                                  k4dirstat *		mainWin )
    : KSettingsPage(parent)
    , _mainWin( mainWin )
    , _treeView( mainWin->treeView() )
    , _maxButtons( KDirStatSettingsMaxColorButton )
{
    // Outer layout box

    QHBoxLayout * outerBox = new QHBoxLayout(this);
    outerBox->setMargin(0);
    // Inner layout box with a column of color buttons

    QGridLayout *grid = new QGridLayout();
    outerBox->addLayout( grid, 1 );
    grid->setColumnStretch( 0, 0 );	// label column - dont' stretch

    for ( int i=1; i < _maxButtons; i++ )
    {
	grid->setColumnStretch( i, 1 );	// all other columns stretch as you like
    }

    for ( int i=0; i < _maxButtons; i++ )
    {
	QString labelText;

	labelText=i18n( "Tree Level %1", i+1);
	_colorLabel[i] = new QLabel( labelText, this );
	grid->addWidget( _colorLabel [i], i, 0 );

	_colorButton[i] = new KColorButton( this );
	_colorButton[i]->setMinimumSize( QSize( 80, 10 ) );
	grid->addWidget(_colorButton[i], i, i+1, 1, _maxButtons - i);
	grid->setRowStretch( i, 1 );
    }


    // Vertical slider

    _slider = new QSlider(Qt::Vertical, this);
    _slider->setMinimum(1);
    _slider->setMaximum(_maxButtons);
    _slider->setPageStep(1);
    _slider->setValue(1);

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

    _treeView->viewport()->repaint();
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



KCleanupPage::KCleanupPage( QWidget *		parent,
                            k4dirstat *	mainWin )
    : KSettingsPage(parent )
    , _mainWin( mainWin )
    , _currentCleanup( 0 )
{
    // Copy the main window's cleanup collection.

    _workCleanupCollection = *mainWin->cleanupCollection();

    // Create layout and widgets.

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setMargin(0);
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

    while ( it.hasNext() )
    {
	_listBox->insert( it.next() );
    }


    // (Re-) Initialize list box.

    // _listBox->resize( _listBox->sizeHint() );
    _listBox->setCurrentRow(0);
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
   : QListWidget( parent )
{
    _selection = 0;

    connect( this,
             SIGNAL( currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
             SLOT  ( selectCleanup   ( QListWidgetItem *) ) );
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
KCleanupListBox::selectCleanup( QListWidgetItem * listBoxItem )
{
    KCleanupListBoxItem * item = (KCleanupListBoxItem *) listBoxItem;
    if(item) {
        _selection = item->cleanup();
        emit selectCleanup( _selection );
    }
}


void
KCleanupListBox::updateTitle( KCleanup * cleanup )
{
    for(int i = 0; i < count(); i ++) {
        KCleanupListBoxItem * item = static_cast<KCleanupListBoxItem *>(this->item(i));
        if ( ! cleanup || item->cleanup() == cleanup )
            item->updateTitle();
    }
}


/*--------------------------------------------------------------------------*/


KCleanupListBoxItem::KCleanupListBoxItem( KCleanupListBox *	listBox,
					  KCleanup *		cleanup )
	: QListWidgetItem( listBox )
    , _cleanup( cleanup )
{
    Q_CHECK_PTR( cleanup );
    setText( cleanup->cleanTitle() );
}


void
KCleanupListBoxItem::updateTitle()
{
    setText( _cleanup->cleanTitle() );
}


/*--------------------------------------------------------------------------*/


KCleanupPropertiesPage::KCleanupPropertiesPage( QWidget *	parent,
                                                k4dirstat *	mainWin )
   : QWidget( parent )
   , _mainWin( mainWin )
{
    QVBoxLayout *outerBox = new QVBoxLayout(this);
    outerBox->setMargin(0);
    outerBox->setSpacing(0);
    // The topmost check box: "Enabled".

    _enabled = new QCheckBox( i18n( "&Enabled" ), this );
    outerBox->addWidget( _enabled, 0 );
    outerBox->addSpacing( 7 );
    outerBox->addStretch();

    connect( _enabled,	SIGNAL( toggled	    ( bool ) ),
	     this,	SLOT  ( enableFields( bool ) ) );


    // All other widgets of this page are grouped together in a
    // separate subwidget so they can all be enabled / disabled
    // together.
    _fields  = new QWidget( this );
    outerBox->addWidget( _fields, 1 );

    QVBoxLayout *fieldsBox = new QVBoxLayout( _fields );


    // Grid layout for the edit fields, their labels, some
    // explanatory text and the "recurse?" check box.

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(4);
    fieldsBox->addLayout( grid, 0 );
    fieldsBox->addStretch();
    fieldsBox->addSpacing( 5 );

    grid->setColumnStretch( 0, 0 ); // column for field labels - dont' stretch
    grid->setColumnStretch( 1, 1 ); // column for edit fields - stretch as you like


    // Edit fields for cleanup action title and command line.

    QLabel *label;
    _title	= new QLineEdit( _fields );					grid->addWidget( _title,   0, 1 );
    _command	= new QLineEdit( _fields );					grid->addWidget( _command, 1, 1 );

    label = new QLabel(i18n("&Title:"), _fields);
    label->setBuddy(_title);
    grid->addWidget(label, 0, 0);

    label = new QLabel(i18n("&Command Line:"), _fields);
    label->setBuddy(_command);
    grid->addWidget(label, 1, 0);

    label = new QLabel( i18n( "%p Full Path" ), _fields );
    grid->addWidget( label, 2, 1 );

    label = new QLabel( i18n( "%n File / Directory Name Without Path" ), _fields );
    grid->addWidget( label, 3, 1 );

    label = new QLabel( i18n( "%t KDE Trash Directory" ), _fields );
    grid->addWidget( label, 4, 1 );


    // "Recurse into subdirs" check box

    _recurse = new QCheckBox( i18n( "&Recurse into Subdirectories" ), _fields );
    grid->addWidget( _recurse, 5, 1 );

    // "Ask for confirmation" check box

    _askForConfirmation = new QCheckBox( i18n( "&Ask for Confirmation" ), _fields );
    grid->addWidget( _askForConfirmation, 6, 1 );


    // The "Works for..." check boxes, grouped together in a button group.

    QGroupBox *worksFor = new QGroupBox( i18n( "Works for..." ), _fields );
    QVBoxLayout *worksForBox = new QVBoxLayout(worksFor);
    fieldsBox->addWidget( worksFor, 0 );
    fieldsBox->addSpacing( 5 );
    fieldsBox->addStretch();

    _worksForDir	= new QCheckBox( i18n( "&Directories"		), worksFor );
    _worksForFile	= new QCheckBox( i18n( "&Files"			), worksFor );
    _worksForDotEntry	= new QCheckBox( i18n( "<Files> P&seudo Entries"), worksFor );

    worksForBox->addWidget( _worksForDir	, 1 );
    worksForBox->addWidget( _worksForFile	, 1 );
    worksForBox->addWidget( _worksForDotEntry	, 1 );

    worksForBox->addSpacing( 5 );
    _worksForProtocols = new QComboBox(worksFor);
    _worksForProtocols->setEditable(false);
    worksForBox->addWidget( _worksForProtocols, 1 );

    _worksForProtocols->insertItem(0, i18n("On Local Machine Only ('file:/' Protocol)"));
    _worksForProtocols->insertItem(1, i18n("Network Transparent (ftp, smb, tar, ...)"));


    // Grid layout for combo boxes at the bottom

    grid = new QGridLayout();
    grid->setSpacing(4);
    fieldsBox->addLayout( grid, 0 );
    fieldsBox->addSpacing( 5 );
    fieldsBox->addStretch();
    int row = 0;


    // The "Refresh policy" combo box

    _refreshPolicy = new QComboBox(_fields);
    _refreshPolicy->setEditable(false);
    grid->addWidget( _refreshPolicy, row, 1 );

    label = new QLabel(i18n( "Refresh &Policy:" ), _fields);
    label->setBuddy(_refreshPolicy);
    grid->addWidget( label, row++, 0 );


    // Caution: The order of those entries must match the order of
    // 'enum RefreshPolicy' in 'kcleanup.h'!
    //
    // I don't like this one bit. The ComboBox should provide something better
    // than mere numeric IDs. One of these days I'm going to rewrite this
    // thing!

    _refreshPolicy->insertItem(0, i18n("No Refresh"));
    _refreshPolicy->insertItem(1, i18n("Refresh This Entry"));
    _refreshPolicy->insertItem(2, i18n("Refresh This Entry's Parent"));
    _refreshPolicy->insertItem(3, i18n("Assume Entry Has Been Deleted"));

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
    _askForConfirmation->setChecked	( cleanup->askForConfirmation() );
    _worksForDir->setChecked		( cleanup->worksForDir()	);
    _worksForFile->setChecked		( cleanup->worksForFile()	);
    _worksForDotEntry->setChecked	( cleanup->worksForDotEntry()	);
    _worksForProtocols->setCurrentIndex	( cleanup->worksLocalOnly() ? 0 : 1 );
    _refreshPolicy->setCurrentIndex	( cleanup->refreshPolicy()	);

    enableFields( cleanup->enabled() );
}


KCleanup
KCleanupPropertiesPage::fields() const
{
    KCleanup cleanup( _id , _command->text(), _title->text(), _mainWin->actionCollection());

    cleanup.setEnabled			( _enabled->isChecked()		   );
    cleanup.setRecurse			( _recurse->isChecked()		   );
    cleanup.setAskForConfirmation	( _askForConfirmation->isChecked() );
    cleanup.setWorksForDir		( _worksForDir->isChecked()	   );
    cleanup.setWorksForFile		( _worksForFile->isChecked()	   );
    cleanup.setWorksLocalOnly		( _worksForProtocols->currentIndex() == 0 ? true : false );
    cleanup.setWorksForDotEntry		( _worksForDotEntry->isChecked()   );
    cleanup.setRefreshPolicy		( (KCleanup::RefreshPolicy) _refreshPolicy->currentIndex() );

    return cleanup;
}


/*--------------------------------------------------------------------------*/


KGeneralSettingsPage::KGeneralSettingsPage( QWidget *		parent,
                                            k4dirstat *	mainWin )
    : KSettingsPage(parent)
    , _mainWin( mainWin )
    , _treeView( mainWin->treeView() )
{
    // Create layout and widgets.

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(5);
    QGroupBox * gbox		= new QGroupBox( i18n( "Directory Reading" ), this );
    layout->addWidget( gbox );
    QVBoxLayout * gboxLayout = new QVBoxLayout(gbox);
    gbox->setLayout(gboxLayout);

    _crossFileSystems		= new QCheckBox( i18n( "Cross &File System Boundaries" ));
    _enableLocalDirReader	= new QCheckBox( i18n( "Use Optimized &Local Directory Read Methods" ));
    gboxLayout->addWidget(_crossFileSystems);
    gboxLayout->addWidget(_enableLocalDirReader);

    connect( _enableLocalDirReader,	SIGNAL( stateChanged( int ) ),
	     this,			SLOT  ( checkEnabledState() ) );

    layout->addSpacing( 10 );
    
    QGroupBox * excludeBox	= new QGroupBox( i18n( "&Exclude Rules" ));
    layout->addWidget( excludeBox );
    QVBoxLayout * excludeBoxLayout = new QVBoxLayout();
    excludeBox->setLayout(excludeBoxLayout);
    _excludeRulesListView	= new QListWidget();
    _excludeRuleContextMenu	= 0;
    excludeBoxLayout->addWidget(_excludeRulesListView);

    QHBoxLayout * buttonBoxLayout = new QHBoxLayout();
    excludeBoxLayout->addLayout(buttonBoxLayout);
    _addExcludeRuleButton = new QPushButton(i18n("&Add"));
    _editExcludeRuleButton = new QPushButton(i18n("&Edit"));
    _deleteExcludeRuleButton = new QPushButton(i18n("&Delete"));
    buttonBoxLayout->addWidget(_addExcludeRuleButton);
    buttonBoxLayout->addWidget(_editExcludeRuleButton);
    buttonBoxLayout->addWidget(_deleteExcludeRuleButton);
    
    _excludeRulesListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_excludeRulesListView, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showExcludeRuleContextMenu(const QPoint&)));

    connect( _addExcludeRuleButton,	SIGNAL( clicked()        ),
	     this,			SLOT  ( addExcludeRule() ) );
    
    connect( _editExcludeRuleButton,	SIGNAL( clicked()         ),
	     this,			SLOT  ( editExcludeRule() ) );
    
    connect( _deleteExcludeRuleButton,	SIGNAL( clicked()           ),
	     this,			SLOT  ( deleteExcludeRule() ) );

    connect( _excludeRulesListView,	SIGNAL( itemDoubleClicked( QListWidgetItem *) ),
	     this, 			SLOT  ( editExcludeRule() ) );
}


KGeneralSettingsPage::~KGeneralSettingsPage()
{
    if ( _excludeRuleContextMenu )
	delete _excludeRuleContextMenu;
}


void
KGeneralSettingsPage::showExcludeRuleContextMenu(const QPoint & localPos)
{
    if ( ! _excludeRuleContextMenu )
    {
        _excludeRuleContextMenu = new QMenu();
        QAction * editAction = new QAction(i18n("&Edit"), _excludeRuleContextMenu);
        connect(editAction, SIGNAL(triggered()), this, SLOT(editExcludeRule()));
        _excludeRuleContextMenu->addAction(editAction);
        QAction * deleteAction = new QAction(i18n("&Delete"), _excludeRuleContextMenu);
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteExcludeRule()));
        _excludeRuleContextMenu->addAction(deleteAction);
    }

    if ( _excludeRuleContextMenu && _excludeRulesListView->currentItem() )
    {
        QPoint pos = _excludeRulesListView->viewport()->mapToGlobal(localPos);
	_excludeRuleContextMenu->move( pos.x(), pos.y() );
	_excludeRuleContextMenu->show();
    }
}


void
KGeneralSettingsPage::apply()
{
    KConfigGroup config = KGlobal::config()->group( "Directory Reading" );

    config.writeEntry( "CrossFileSystems",	_crossFileSystems->isChecked()		);
    config.writeEntry( "EnableLocalDirReader", _enableLocalDirReader->isChecked()	);

    config = KGlobal::config()->group( "Exclude" );
    //config.setGroup( "Exclude" );
    
    QStringList excludeRulesStringList;
    KExcludeRules::excludeRules()->clear();

    for(int i = 0; i < _excludeRulesListView->count(); i++) {
        QListWidgetItem * item = _excludeRulesListView->item(i);
        QString ruleText = item->text();
        excludeRulesStringList.append( ruleText );
        // qDebug() << "Adding exclude rule " << ruleText << endl;
        KExcludeRules::excludeRules()->add( new KExcludeRule( QRegExp( ruleText ) ) );
    }

    config.writeEntry( "ExcludeRules", excludeRulesStringList );
}


void
KGeneralSettingsPage::revertToDefaults()
{
    _crossFileSystems->setChecked( false );
    _enableLocalDirReader->setChecked( true );
    _excludeRulesListView->clear();
    _editExcludeRuleButton->setEnabled( false );
    _deleteExcludeRuleButton->setEnabled( false );
}


void
KGeneralSettingsPage::setup()
{
    KConfigGroup config = KGlobal::config()->group("Directory Reading");

    _crossFileSystems->setChecked	( config.readEntry( "CrossFileSystems"	, false) );
    _enableLocalDirReader->setChecked	( config.readEntry( "EnableLocalDirReader" , true ) );
    _excludeRulesListView->clear();

    foreach(KExcludeRule * excludeRule, KExcludeRules::excludeRules()->rules()) {
        QListWidgetItem * n = new QListWidgetItem(_excludeRulesListView);
        n->setText(excludeRule->regexp().pattern());
    }
    
    checkEnabledState();
}


void
KGeneralSettingsPage::checkEnabledState()
{
    _crossFileSystems->setEnabled( _enableLocalDirReader->isChecked() );

    int excludeRulesCount = _excludeRulesListView->count();
    
    _editExcludeRuleButton->setEnabled  ( excludeRulesCount > 0 );
    _deleteExcludeRuleButton->setEnabled( excludeRulesCount > 0 );
}


void
KGeneralSettingsPage::addExcludeRule()
{
    bool ok;
    QString text = QInputDialog::getText(this, i18n( "New exclude rule" ),
					  i18n( "Regular expression for new exclude rule:" ),
					  QLineEdit::Normal,
					  QString::null,
					  &ok);
    if ( ok && ! text.isEmpty() )
    {
        QListWidgetItem * l = new QListWidgetItem(_excludeRulesListView);
        l->setText(text);
        _excludeRulesListView->addItem(l);
    }
    
    checkEnabledState();
}


void
KGeneralSettingsPage::editExcludeRule()
{
    QListWidgetItem * item = _excludeRulesListView->currentItem();

    if ( item )
    {
	bool ok;
	QString text = QInputDialog::getText(this, i18n( "Edit exclude rule" ),
					      i18n( "Exclude rule (regular expression):" ),
					      QLineEdit::Normal,
						  item->text(),
						  &ok);
	if ( ok )
	{
	    if ( text.isEmpty() )
		_excludeRulesListView->takeItem(_excludeRulesListView->currentRow());
	    else
			item->setText(text);
	}
    }
    
    checkEnabledState();
}


void KGeneralSettingsPage::deleteExcludeRule()
{
    QListWidgetItem * item = _excludeRulesListView->currentItem();

    if ( item )
    {
        QString excludeRule = item->text();
        int result = KMessageBox::questionYesNo( this,
            i18n( "Really delete exclude rule \"%1\"?" ).arg( excludeRule ),
            i18n( "Delete?" ) ); // Window title
        if ( result == KMessageBox::Yes )
        {
            _excludeRulesListView->takeItem(_excludeRulesListView->currentRow());
        }
    }

    checkEnabledState();
}


/*--------------------------------------------------------------------------*/


KTreemapPage::KTreemapPage(QWidget * parent, k4dirstat * mainWin)
    : KSettingsPage(parent)
    , _mainWin( mainWin )
{
    // qDebug() << k_funcinfo << endl;

    QVBoxLayout * layout = new QVBoxLayout(this); // parent
    setLayout(layout);
    _squarify = new QCheckBox(i18n("S&quarify Treemap"), this);
    _doCushionShading = new QCheckBox(i18n("Use C&ushion Shading"), this);
    layout->addWidget(_squarify);
    layout->addWidget(_doCushionShading);

    // Cushion parameters

    QGroupBox * gbox	= new QGroupBox( i18n( "Cushion Parameters" ), this );
    layout->addWidget(gbox);
    _cushionParams	= gbox;
    QLabel * label	= new QLabel( i18n( "Ambient &Light" ));
    _ambientLight	= new QSlider (Qt::Horizontal);
    _ambientLight->setMinimum(MinAmbientLight);
    _ambientLight->setMaximum(MaxAmbientLight);
    _ambientLight->setPageStep(10);
    _ambientLight->setValue(DefaultAmbientLight);
    _ambientLightSB	= new QSpinBox();
    _ambientLightSB->setMinimum(MinAmbientLight);
    _ambientLightSB->setMaximum(MaxAmbientLight);
    label->setBuddy( _ambientLightSB );

    QGridLayout * cushionLayout = new QGridLayout();
    gbox->setLayout(cushionLayout);
    cushionLayout->setColumnStretch(1, 1);
    cushionLayout->addWidget(label, 0, 0, 1, 1);
    cushionLayout->addWidget(_ambientLight, 0, 1, 1, 2);
    cushionLayout->addWidget(_ambientLightSB, 0, 3, 1, 1);

    label = new QLabel( i18n( "&Height Scale" ));
    _heightScalePercent = new QSlider(Qt::Horizontal);
    _heightScalePercent->setMinimum(MinHeightScalePercent);
    _heightScalePercent->setMaximum(MaxHeightScalePercent);
    _heightScalePercent->setPageStep(10);
    _heightScalePercent->setValue(DefaultHeightScalePercent);
    _heightScalePercent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _heightScalePercentSB = new QSpinBox();
    _heightScalePercentSB->setMinimum(MinHeightScalePercent);
    _heightScalePercentSB->setMaximum(MaxHeightScalePercent);
    label->setBuddy( _heightScalePercentSB );

    cushionLayout->addWidget(label, 1, 0, 1, 1);
    cushionLayout->addWidget(_heightScalePercent, 1, 1, 1, 2);
    cushionLayout->addWidget(_heightScalePercentSB, 1, 3, 1, 1);

    _ensureContrast	= new QCheckBox( i18n( "Draw Lines if Lo&w Contrast"));

    _forceCushionGrid	= new QCheckBox( i18n( "Always Draw &Grid"));
    _cushionGridColorL	= new QLabel(i18n( "Gr&id Color: "));
    _cushionGridColor	= new KColorButton();
    _cushionGridColorL->setBuddy( _cushionGridColor );
    cushionLayout->addWidget(_forceCushionGrid, 4, 0, 1, 2);
    cushionLayout->addWidget(_ensureContrast, 3, 0, 1, 2);
    cushionLayout->addWidget(_cushionGridColorL, 4, 2, 1, 1, Qt::AlignRight);
    cushionLayout->addWidget(_cushionGridColor, 4, 3, 1, 1);

    // Plain treemaps parameters

    _plainTileParams	= new QGroupBox( i18n( "Colors for Plain Treemaps" ), this );
    layout->addWidget(_plainTileParams);
    QHBoxLayout * plainTileLayout = new QHBoxLayout();
    _plainTileParams->setLayout(plainTileLayout);
    label = new QLabel(i18n("&Files: "));
    plainTileLayout->addWidget(label);
    _fileFillColor = new KColorButton();
    plainTileLayout->addWidget(_fileFillColor);
    label->setBuddy( _fileFillColor );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    label = new QLabel( "	   " + i18n( "&Directories: " ));
    _dirFillColor = new KColorButton();
    label->setBuddy( _dirFillColor );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    plainTileLayout->addWidget(label);
    plainTileLayout->addWidget(_dirFillColor);

    label		= new QLabel( i18n( "Gr&id: " ));
    _outlineColor	= new KColorButton();
    label->setBuddy( _outlineColor );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    plainTileLayout->addWidget(label);
    plainTileLayout->addWidget(_outlineColor);


    // Misc

    QWidget * gridBox	= new QWidget( this );
    layout->addWidget(gridBox);
    QGridLayout * grid = new QGridLayout(gridBox);
    grid->setColumnStretch( 0, 0 ); // (col, stretch) don't stretch this column
    grid->setColumnStretch( 1, 0 ); // don't stretch
    grid->setColumnStretch( 2, 1 ); // stretch this as you like

    label		= new QLabel( i18n( "Highlight R&ectangle: " ), gridBox );
    _highlightColor	= new KColorButton( gridBox );
    label->setBuddy( _highlightColor );

    grid->addWidget( label,		0, 0 );
    grid->addWidget( _highlightColor,	0, 1 );


    label		= new QLabel( i18n( "Minim&um Treemap Tile Size: " ), gridBox );
    _minTileSize = new QSpinBox(gridBox);
    _minTileSize->setMinimum(0);
    _minTileSize->setMaximum(30);
    _minTileSize->setSingleStep(1);
    label->setBuddy( _minTileSize );

    grid->addWidget( label,		1, 0 );
    grid->addWidget( _minTileSize,	1, 1 );

    _autoResize		= new QCheckBox( i18n( "Auto-&Resize Treemap" ), this );
    layout->addWidget(_autoResize);


    // Connections

    connect( _ambientLight,		SIGNAL( valueChanged(int) ),
	     _ambientLightSB,		SLOT  ( setValue    (int) ) );

    connect( _ambientLightSB,		SIGNAL( valueChanged(int) ),
	     _ambientLight,		SLOT  ( setValue    (int) ) );


    connect( _heightScalePercent,	SIGNAL( valueChanged(int) ),
	     _heightScalePercentSB,	SLOT  ( setValue    (int) ) );

    connect( _heightScalePercentSB,	SIGNAL( valueChanged(int) ),
	     _heightScalePercent,	SLOT  ( setValue    (int) ) );


    connect( _doCushionShading, SIGNAL( stateChanged( int ) ), this, SLOT( checkEnabledState() ) );
    connect( _forceCushionGrid, SIGNAL( stateChanged( int ) ), this, SLOT( checkEnabledState() ) );

    checkEnabledState();
}


KTreemapPage::~KTreemapPage()
{
    // NOP
}


void
KTreemapPage::apply()
{
    KConfigGroup config = KGlobal::config()->group( "Treemaps" );

    config.writeEntry( "Squarify",		_squarify->isChecked()			);
    config.writeEntry( "CushionShading",	_doCushionShading->isChecked()		);
    config.writeEntry( "AmbientLight",		_ambientLight->value()			);
    config.writeEntry( "HeightScaleFactor",	_heightScalePercent->value() / 100.0	);
    config.writeEntry( "EnsureContrast",	_ensureContrast->isChecked()		);
    config.writeEntry( "ForceCushionGrid",	_forceCushionGrid->isChecked()		);
    config.writeEntry( "MinTileSize",		_minTileSize->value()			);
    config.writeEntry( "AutoResize",		_autoResize->isChecked()		);
    config.writeEntry( "CushionGridColor",	_cushionGridColor->color()		);
    config.writeEntry( "OutlineColor",		_outlineColor->color()			);
    config.writeEntry( "FileFillColor",	_fileFillColor->color()			);
    config.writeEntry( "DirFillColor",		_dirFillColor->color()			);
    config.writeEntry( "HighlightColor",	_highlightColor->color()		);

    if ( treemapView() )
    {
	treemapView()->readConfig();
	treemapView()->rebuildTreemap();
    }
}


void
KTreemapPage::revertToDefaults()
{
    _squarify->setChecked( true );
    _doCushionShading->setChecked( true );

    _ambientLight->setValue( DefaultAmbientLight );
    _heightScalePercent->setValue( DefaultHeightScalePercent );
    _ensureContrast->setChecked( true );
    _forceCushionGrid->setChecked( false );
    _minTileSize->setValue( DefaultMinTileSize );
    _autoResize->setChecked( true );

    _cushionGridColor->setColor ( QColor( 0x80, 0x80, 0x80 ) );
    _outlineColor->setColor	( QColor(Qt::black)          );
    _fileFillColor->setColor	( QColor( 0xde, 0x8d, 0x53 ) );
    _dirFillColor->setColor	( QColor( 0x10, 0x7d, 0xb4 ) );
    _highlightColor->setColor	( QColor(Qt::red)	     );
}


void
KTreemapPage::setup()
{
    KConfigGroup config = KGlobal::config()->group("Treemaps");

    _squarify->setChecked		( config.readEntry( "Squarify"		, true	) );
    _doCushionShading->setChecked	( config.readEntry( "CushionShading"	, true	) );

    _ambientLight->setValue		( config.readEntry( "AmbientLight"		  , DefaultAmbientLight       ) );
    _heightScalePercent->setValue( (int) ( 100 *  config.readEntry ( "HeightScaleFactor", DefaultHeightScaleFactor  ) ) );
    _ensureContrast->setChecked		( config.readEntry( "EnsureContrast"	, true	) );
    _forceCushionGrid->setChecked	( config.readEntry( "ForceCushionGrid"	, false ) );
    _minTileSize->setValue		( config.readEntry ( "MinTileSize"		, DefaultMinTileSize ) );
    _autoResize->setChecked		( config.readEntry( "AutoResize"		, true	) );

    _cushionGridColor->setColor ( readColorEntry( config, "CushionGridColor"	, QColor( 0x80, 0x80, 0x80 ) ) );
    _outlineColor->setColor	( readColorEntry( config, "OutlineColor"	, QColor(Qt::black)	     ) );
    _fileFillColor->setColor	( readColorEntry( config, "FileFillColor"	, QColor( 0xde, 0x8d, 0x53 ) ) );
    _dirFillColor->setColor	( readColorEntry( config, "DirFillColor"	, QColor( 0x10, 0x7d, 0xb4 ) ) );
    _highlightColor->setColor	( readColorEntry( config, "HighlightColor"	, QColor(Qt::red)	     ) );

    _ambientLightSB->setValue( _ambientLight->value() );
    _heightScalePercentSB->setValue( _heightScalePercent->value() );

    checkEnabledState();
}


void
KTreemapPage::checkEnabledState()
{
    _cushionParams->setEnabled( _doCushionShading->isChecked() );
    _plainTileParams->setEnabled( ! _doCushionShading->isChecked() );

    if ( _doCushionShading->isChecked() )
    {
	_cushionGridColor->setEnabled ( _forceCushionGrid->isChecked() );
	_cushionGridColorL->setEnabled( _forceCushionGrid->isChecked() );
	_ensureContrast->setEnabled   ( ! _forceCushionGrid->isChecked() );
    }
}


QColor
KTreemapPage::readColorEntry( KConfigGroup config, const char * entryName, QColor defaultColor )
{
    return config.readEntry( entryName, defaultColor );
}



void
addHStretch( QWidget * parent )
{
    QWidget * stretch = new QWidget( parent );
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Minimum);
    sp.setHorizontalStretch(1);
    sp.setVerticalStretch(0);
    stretch->setSizePolicy(sp);
}


void
addVStretch( QWidget * parent )
{
    QWidget * stretch = new QWidget( parent );
    QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Expanding);
    sp.setHorizontalStretch(0);
    sp.setVerticalStretch(1);
    stretch->setSizePolicy(sp);
}

// EOF
