
/*
 *   File name:	kfeedback.cpp
 *   Summary:	User feedback form
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2003-01-30
 */


#include <qheader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qhbox.h>

#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kurl.h>

#include "kfeedback.h"


KFeedbackDialog::KFeedbackDialog( const QString & feedbackMailAddress,
				  const QString & helpTopic		)
    : KDialogBase( Plain,					// dialogFace
		   i18n( "Feedback" ),				// caption
		   Apply | Cancel
		   | ( helpTopic.isEmpty() ? 0 : (int) Help ),	// buttonMask
		   Apply )					// defaultButton
{
    QVBoxLayout * layout = new QVBoxLayout( plainPage(), 0, spacingHint() );
    setButtonApplyText( i18n( "&Mail this..." ) );

    if ( ! helpTopic.isEmpty() )
	setHelp( helpTopic );

    _form = new KFeedbackForm( feedbackMailAddress, plainPage() );
    CHECK_PTR( _form );

    layout->addWidget( _form );
    checkSendButton();

    connect( this,  SIGNAL( applyClicked() ),
	     _form, SLOT  ( sendMail()  ) );

    connect( _form, SIGNAL( mailSent() 	),
	     this,  SLOT  ( hide()	) );

    connect( _form, SIGNAL( mailSent()	),
	     this,  SIGNAL( mailSent()  ) );

    connect( _form, SIGNAL( checkComplete()   ),
	     this,  SLOT  ( checkSendButton() ) );
}


KFeedbackDialog::~KFeedbackDialog()
{
    // NOP
}


void
KFeedbackDialog::checkSendButton()
{
    enableButtonApply( _form->readyToSend() );
}





KFeedbackForm::KFeedbackForm( const QString &	feedbackMailAddress,
			      QWidget *		parent )
    : QVBox( parent )
    , _feedbackMailAddress( feedbackMailAddress )
{
    //
    // Explanation above the question list
    //
    
    QLabel * label = new QLabel( i18n( "<p><b>Please tell us your opinion about this program.</b></p>"
				       "<p>You will be able to review everything in your mailer "
				       "before any mail is sent.<br>"
				       "Nothing will be sent behind your back.</p>"
				       ), this );
    //
    // Question list
    //
    
    _questionList = new KFeedbackQuestionList( this );
    CHECK_PTR( _questionList );
    
    connect( _questionList, SIGNAL( checkComplete()     ),
	     this,	    SLOT  ( slotCheckComplete() ) );


    //
    // Explanation below the question list
    //

    QHBox * hbox = new QHBox( this );
    CHECK_PTR( hbox );

    QSizePolicy pol( QSizePolicy::Fixed, QSizePolicy::Fixed ); // hor / vert
    
    label = new QLabel( i18n( "Questions marked with " ), hbox ); 
    CHECK_PTR( label );
    label->setSizePolicy( pol );

    label = new QLabel( hbox );
    CHECK_PTR( label );
    label->setPixmap( KGlobal::iconLoader()->loadIcon( "edit", KIcon::Small ) );
    label->setSizePolicy( pol );

    label = new QLabel( i18n( " must be answered before a mail can be sent.") , hbox );
    CHECK_PTR( label );
    label->setSizePolicy( pol );
    
    new QWidget( hbox );	// Fill any leftover space to the right.


    //
    // Free-text comment field
    //

    label = new QLabel( "\n" + i18n( "&Additional Comments:" ), this );	CHECK_PTR( label );
    _comment = new QMultiLineEdit( this );				CHECK_PTR( _comment );

    label->setBuddy( _comment );
#if (QT_VERSION < 300)
    _comment->setFixedVisibleLines( 5 );
#endif
    _comment->setWordWrap( QMultiLineEdit::FixedColumnWidth );
    _comment->setWrapColumnOrWidth( 70 );
}


KFeedbackForm::~KFeedbackForm()
{
    // NOP
}


void
KFeedbackForm::sendMail()
{
    //
    // Build mail subject
    //

    QString subject;

    const KAboutData * aboutData = KGlobal::instance()->aboutData();

    if ( aboutData )
	subject = aboutData->programName() + "-" + aboutData->version();
    else
	subject = kapp->name();

    subject = "[kde-feedback] " + subject + " user feedback";


    //
    // Build mail body
    //

    QString body = subject + "\n\n"
	+ formatComment()
	+ _questionList->result();


    //
    // Build "mailto:" URL from all this
    //

    KURL mail;
    mail.setProtocol( "mailto" );
    mail.setPath( _feedbackMailAddress );
    mail.setQuery( "?subject="	+ KURL::encode_string( subject ) +
		   "&body="	+ KURL::encode_string( body ) );

    // TODO: Check for maximum command line length.
    //
    // The hard part with this is how to get this from all that 'autoconf'
    // stuff into 'config.h' or some other include file without hardcoding
    // anything - this is too system dependent.


    //
    // Actually send mail
    //

    kapp->invokeMailer( mail );

    emit mailSent();
}


void
KFeedbackForm::slotCheckComplete()
{
    emit checkComplete();
}


QString
KFeedbackForm::formatComment()
{
    QString result = _comment->text();

    if ( ! result.isEmpty() )
    {
	result = "<comment>\n" + result + "\n</comment>\n\n";
    }

    return result;
}


bool
KFeedbackForm::readyToSend()
{
    return _questionList->isComplete();
}






KFeedbackQuestionList::KFeedbackQuestionList( QWidget *parent )
    : QListView( parent )
{
    addColumn( "" );
    header()->hide();
}


KFeedbackQuestionList::~KFeedbackQuestionList()
{
    // NOP
}


bool
KFeedbackQuestionList::isComplete()
{
    KFeedbackQuestion * question = firstQuestion();

    while ( question )
    {
	if ( question->isRequired() && ! question->isAnswered() )
	    return false;

	question = question->nextQuestion();
    }

    return true;
}


QString
KFeedbackQuestionList::result()
{
    QString res;
    KFeedbackQuestion * question = firstQuestion();

    while ( question )
    {
	res += question->result();

	question = question->nextQuestion();
    }

    return res;
}


KFeedbackQuestion *
KFeedbackQuestionList::addQuestion( const QString & 	text,
				    const QString &	id,
				    bool		exclusiveAnswer,
				    bool 		required )
{
    KFeedbackQuestion * question = new KFeedbackQuestion( this, text, id,
							  exclusiveAnswer,
							  required );
    CHECK_PTR( question );

    return question;
}


void
KFeedbackQuestionList::addYesNoQuestion( const QString & 	text,
					 const QString &	id,
					 bool			required )
{

    KFeedbackQuestion * question = new KFeedbackQuestion( this, text, id,
							  true,	// exclusive
							  required );
    CHECK_PTR( question );
    question->addAnswer( i18n( "yes" ), "yes" );
    question->addAnswer( i18n( "no"  ), "no"  );
}


void
KFeedbackQuestionList::questionAnswered()
{
    emit checkComplete();
}

void
KFeedbackQuestionList::questionAdded( KFeedbackQuestion * question)
{
    if ( question->isRequired() )
	emit checkComplete();
}





static int nextNo = 0;

KFeedbackQuestion::KFeedbackQuestion( KFeedbackQuestionList *	parent,
				      const QString & 		text,
				      const QString &		id,
				      bool			exclusiveAnswer,
				      bool			required,
				      bool			open	)
    : QCheckListItem( parent, text )
    , _id( id )
    , _exclusiveAnswer( exclusiveAnswer )
    , _required( required )
{
    if ( required )
    {
	setPixmap( 0, KGlobal::iconLoader()->loadIcon( "edit", KIcon::Small ) );
    }

    setOpen( open );
    _no = nextNo++;

    parent->questionAdded( this );
}


void
KFeedbackQuestion::addAnswer( const QString & text,
			      const QString & id   )
{
    new KFeedbackAnswer( this, text, id, _exclusiveAnswer );
}


bool
KFeedbackQuestion::isAnswered()
{
    if ( ! _exclusiveAnswer )
    {
	/**
	 * If any number of answers is permitted for this question, this
	 * question is always considered to be answered.
	 **/

	return true;
    }


    /**
     * If this question requires an exclusive answer, exactly one of them
     * should be checked. We don't need to bother about more than one being
     * checked here - QListView does that for us.
     **/

    KFeedbackAnswer *answer = firstAnswer();

    while ( answer )
    {
	if ( answer->isChecked() )
	    return true;

	answer = answer->nextAnswer();
    }

    return false;
}


QString
KFeedbackQuestion::result()
{
    QString res;
    int answers = 0;

    KFeedbackAnswer *answer = firstAnswer();

    while ( answer )
    {
	if ( answer->isChecked() )
	{
	    res += _id + "=\"" + answer->id() + "\"\n";
	    answers++;
	}

	answer = answer->nextAnswer();
    }

    if ( answers > 1 )
    {
	res = "\n" + res + "\n";
    }

    return res;
}


QString
KFeedbackQuestion::text()
{
    return QCheckListItem::text(0);
}


QString
KFeedbackQuestion::key( int, bool ) const
{
    QString no;
    no.sprintf( "%08d", _no );

    return no;
}


KFeedbackQuestionList *
KFeedbackQuestion::questionList() const
{
    return dynamic_cast<KFeedbackQuestionList *>( listView() );
}







KFeedbackAnswer::KFeedbackAnswer( KFeedbackQuestion * 	parent,
				  const QString &	text,
				  const QString &	id,
				  bool			exclusive )
    : QCheckListItem( parent,
		      text,
		      exclusive
		      ? QCheckListItem::RadioButton
		      : QCheckListItem::CheckBox )
    , _id( id )
{
    _no = nextNo++;
}


QString
KFeedbackAnswer::text()
{
    return QCheckListItem::text(0);
}


QString
KFeedbackAnswer::key( int, bool ) const
{
    QString no;
    no.sprintf( "%08d", _no );

    return no;
}


void
KFeedbackAnswer::stateChange( bool newState )
{
    if ( newState && question()->isRequired() )
    {
	KFeedbackQuestionList * list = question()->questionList();

	if ( list )
	    list->questionAnswered();
    }
}



// EOF
