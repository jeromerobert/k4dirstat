/*
 *   File name:	kfeedback.h
 *   Summary:	User feedback form and mailing utilities
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-02-24
 *
 *   $Id: kfeedback.h,v 1.2 2002/02/25 10:49:07 hundhammer Exp $
 *
 */


#ifndef KFeedback_h
#define KFeedback_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qlistview.h>
#include <qvbox.h>
#include <kdialogbase.h>


#ifndef NOT_USED
#    define NOT_USED(PARAM)	( (void) (PARAM) )
#endif


class KFeedbackForm;
class KFeedbackQuestionList;
class KFeedbackQuestion;
class KFeedbackAnswer;
class QMultiLineEdit;


/**
 * Dialog containing a @ref KFeedbackForm and all the infrastructure for
 * sending a mail etc.
 **/
class KFeedbackDialog: public KDialogBase
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    KFeedbackDialog( const QString & feedbackMailAddress,
		     const QString & helpTopic = QString::null );


    /**
     * Destructor.
     **/
    virtual ~KFeedbackDialog();


    /**
     * Returns the internal @KFeedbackForm
     **/
    KFeedbackForm *form() { return _form; }


public slots:

    /**
     * Check if sufficient information is available to send a mail now and
     * enable / disable the "send mail" button accordingly.
     **/
    void checkSendButton();


signals:
    /**
     * Emitted when the user has sent the feedback mail - i.e. when he clicked
     * on the "Send mail" button and the mail has successfully been forwarded
     * to the mailer. He can still choose not to send the mail from within the
     * mailer, though.
     **/
    void mailSent();


protected:

    KFeedbackForm *	_form;
};


/**
 * User feedback form:
 *
 * User is asked a list of questions, the answers of which will be sent via
 * mail back to a feedback mail address.
 **/
class KFeedbackForm: public QVBox
{
    Q_OBJECT

public:
    /**
     * Constructor.
     **/
    KFeedbackForm( const QString &	feedbackMailAddress,
		   QWidget *		parent );

    /**
     * Destructor.
     **/
    virtual ~KFeedbackForm();


public slots:

    /**
     * Compose a mail from the user's answers and send it to the feedback mail
     * address passed to the constructor.
     *
     * This method will check with @ref readyToSend() if the mail can be sent
     * with the questions answered until now and prompt the user to answer more
     * questions if not.
     *
     * Connect the @ref mailSent() signal if you are interested when exactly
     * all this was successful.
     **/
     virtual void sendMail();


public:

    /**
     * Checks if the mail is ready to send, i.e. if all required fields are
     * filled.
     **/
    virtual bool readyToSend();

    /**
     * Returns the @ref KFeedbackQuestionList .
     **/
    KFeedbackQuestionList * questionList() { return _questionList; }


signals:
    /**
     * Emitted when the user has sent the feedback mail - i.e. when he clicked
     * on the "Send mail" button and the mail has successfully been forwarded
     * to the mailer. He can still choose not to send the mail from within the
     * mailer, though.
     **/
    void mailSent();

    /**
     * Emitted when it is time to check for completeness of all information in
     * this form: Either when a new question is added or when a question is
     * answered.
     **/
    void checkComplete();


protected slots:
    /**
     * Check for completeness of this form.
     **/
    void slotCheckComplete();


protected:

    /**
     * Format the "personal comments" field for sending mail.
     **/
    QString formatComment();


    QString			_feedbackMailAddress;
    KFeedbackQuestionList *	_questionList;
    QMultiLineEdit *		_comment;
};



/**
 * List of feedback questions presented in a @ref QListView widget.
 **/
class KFeedbackQuestionList: public QListView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    KFeedbackQuestionList( QWidget *parent );

    /**
     * Destructor.
     **/
    virtual ~KFeedbackQuestionList();

    /**
     * Returns whether or not this question list is answered satisfactorily,
     * i.e. if all questions marked as "required" are answered.
     **/
    virtual bool isComplete();

    /**
     * The result of all answered questions in ASCII.
     **/
    QString result();

    /**
     * Add a yes/no question to the list.
     *
     * 'text' is the text the user will see (in his native language).
     *
     * 'id' is what will be sent with the feedback mail, thus it should be
     * unique within the application, yet human readable (preferably English)
     * and not contain any weird characters that might confuse scripts that are
     * later used to automatically parse those mails.
     * Examples: "would_recommend_to_a_friend"
     *
     * Set 'required' to 'true' if answering this question is required to
     * successfully complete this form.
     *
     * Returns a pointer to this question so you can add answers.
     **/

    KFeedbackQuestion * addQuestion( const QString & 	text,
				     const QString &	id,
				     bool		exclusiveAnswer	= true,
				     bool 		required	= false );

    /**
     * Add a yes/no question to the list.
     **/
    void addYesNoQuestion( const QString & 	text,
			   const QString &	id,
			   bool 		required = false );

    /**
     * Returns the first question of that list.
     * Use @ref KFeedbackQuestion::next() to get the next one.
     **/
    KFeedbackQuestion * firstQuestion() const
	{ return (KFeedbackQuestion *) QListView::firstChild(); }

    /**
     * Notify the list that another question has been answered.
     * Emits the @ref checkComplete() signal when all required questions are
     * answered.
     **/
    void questionAnswered();

    /**
     * Notify the list that another question has been added.
     * Emits the @ref checkComplete() signal when a required question is
     * added.
     **/
    void questionAdded( KFeedbackQuestion * question );

signals:
    /**
     * Emitted when all required questions are answered.
     **/
    void checkComplete();
};


/**
 * A user feedback question to be inserted into a @ref KFeedbackQuestionList.
 **/
class KFeedbackQuestion: public QCheckListItem
{
public:

    /**
     * Constructor.
     *
     * The parent @ref KFeedbackQuestionList assumes ownership of this object,
     * so don't delete it unless you want to delete it from the question list
     * as well.
     *
     * 'text' is the text the user will see (in his native language).
     *
     * 'id' is what will be sent with the feedback mail, thus it should be
     * unique within the application, yet human readable (preferably English)
     * and not contain any weird characters that might confuse scripts that are
     * later used to automatically parse those mails.
     * Examples: "features_not_liked", "stability"
     *
     * Set 'required' to 'true' if answering this question is required to
     * successfully complete this form.
     *
     * Set 'exclusiveAnswer' to 'true' if only one of all answers may be
     * checked at any one time, to 'false' if multiple answers are allowed.
     **/
    KFeedbackQuestion( KFeedbackQuestionList *	parent,
		       const QString & 		text,
		       const QString &		id,
		       bool			exclusiveAnswer	= true,
		       bool			required	= false,
		       bool			open		= true );

    /**
     * Add an answer to this question. Again, 'text' is what the user will see
     * (translated to his native language), 'id' is what you will get back with
     * the mail. The answer IDs need only be unique for that question; answers
     * to other questions may have the same ID.
     **/
    void addAnswer( const QString & text,
		    const QString & id   );

    /**
     * Returns if answering this question is required.
     **/
    bool isRequired() { return _required; }

    /**
     * Returns if this question is answered satisfactorily.
     **/
    bool isAnswered();

    /**
     * The result of this question in ASCII, e.g.
     *		recommend="yes"
     * or
     *		features_i_like="builtin_tetris"
     *		features_i_like="pink_elephant"
     *		features_i_like="studlycapslyfier"
     **/
    QString result();

    /**
     * Return this question's ID.
     **/
    QString id() { return _id; }

    /**
     * Return this question's text.
     **/
    QString text();

    /**
     * Returns whether or not this question requires an exclusive answer.
     **/
    bool exclusiveAnswer() { return _exclusiveAnswer; }


    /**
     * Returns the sort key.
     *
     * Reimplemented from @ref QListViewItem to maintain insertion order.
     **/
    virtual QString key( int column, bool ascending ) const;

    /**
     * Returns the next question or 0 if there is no more.
     **/
    KFeedbackQuestion * nextQuestion() const
	{ return (KFeedbackQuestion *) QListViewItem::nextSibling(); }

    /**
     * Returns the first possible answer to this question.
     * Use @ref KFeedbackAnswer::nextAnswer() to get the next one.
     **/
    KFeedbackAnswer * firstAnswer() const
	{ return (KFeedbackAnswer *) QListViewItem::firstChild(); }

    /**
     * Returns the @ref KFeedbackQuestionList this question belongs to or 0 if
     * the parent is no @ref KFeedbackQuestionList.
     **/
    KFeedbackQuestionList * questionList() const;


protected:

    QString	_id;
    bool	_exclusiveAnswer;
    bool	_required;
    int		_no;
};


class KFeedbackAnswer: public QCheckListItem
{
public:
    /**
     * Constructor.
     *
     * 'exclusive' tells the type of answer: One of many allowed or any number
     * of many.
     **/
    KFeedbackAnswer( KFeedbackQuestion * 	parent,
		     const QString &		text,
		     const QString &		id,
		     bool			exclusive = true );

    /**
     * Return this answer's ID.
     **/
    QString id() { return _id; }

    /**
     * Return this answer's text.
     **/
    QString text();

    /**
     * Returns whether or not this is an exclusive answer.
     **/
    bool isExclusive() { return _exclusive; }

    /**
     * Returns whether or not this answer is checked.
     **/
    bool isChecked() { return QCheckListItem::isOn(); }

    /**
     * Returns the next possible answer or 0 if there is no more.
     **/
    KFeedbackAnswer * nextAnswer() const
	{ return (KFeedbackAnswer *) QListViewItem::nextSibling(); }

    /**
     * Returns the question to this answer.
     **/
    KFeedbackQuestion * question() const
	{ return (KFeedbackQuestion *) QListViewItem::parent(); }

    /**
     * Returns the sort key.
     *
     * Reimplemented from @ref QListViewItem to maintain insertion order.
     **/
    virtual QString key( int column, bool ascending ) const;


    /**
     * On/off change.
     *
     * Reimplemented from @ref QCheckListItem to monitor answering required
     * questions. This method notifies the @ref KFeedbackQuestionList whenever
     * a required question is being answered.
     **/
    virtual void stateChange( bool newState );

protected:

    QString	_id;
    bool	_exclusive;
    int		_no;
};



#endif // KFeedback_h


// EOF
