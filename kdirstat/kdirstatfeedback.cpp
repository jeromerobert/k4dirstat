/*
 *   File name:	kdirstatfeedback.cpp
 *   Summary:	User feedback questions for KDirStat
 *   License:	GPL - See file COPYING for details.
 *
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *
 *   Updated:	2002-02-24
 *
 *   $Id: kdirstatfeedback.cpp,v 1.4 2002/02/25 10:49:07 hundhammer Exp $
 *
 */


#include <klocale.h>

#include "kdirstatapp.h"
#include "kfeedback.h"



void
KDirStatApp::sendFeedbackMail()
{
    if ( ! _feedbackDialog )
    {
	_feedbackDialog = new KFeedbackDialog( "sh@suse.de", "feedback_mail" );
	CHECK_PTR( _feedbackDialog );

	connect( _feedbackDialog->form(), SIGNAL( mailSent() ),
		 this, SLOT( feedbackMailSent() ) );

	KFeedbackQuestionList * list = _feedbackDialog->form()->questionList();

	KFeedbackQuestion * question =
	    list->addQuestion( i18n( "What is your general opinion about this program?" ), "general_opinion", true, true );

	question->addAnswer( i18n( "It's one of my favourites" 			), 	"1/8_favourite" 		);
	question->addAnswer( i18n( "I like it" 					), 	"2/8_like_it" 		);
	question->addAnswer( i18n( "It's sometimes useful"			), 	"3/8_sometimes_useful" 	);
	question->addAnswer( i18n( "It's average" 				), 	"4/8_average" 		);
	question->addAnswer( i18n( "Nice try, but this could be done better"	), 	"5/8_nice_try" 		);
	question->addAnswer( i18n( "It's poor"	 				), 	"6/8_poor"		);
	question->addAnswer( i18n( "It's useless"				), 	"7/8_useless" 		);
	question->addAnswer( i18n( "It's crap" 					), 	"8/8_crap"		);

	question = list->addQuestion( i18n( "Which features of this program do you like?" ), "features_liked", false );
	addFeatureList( question );

	question = list->addQuestion( i18n( "Which features don't you like?" ), 	"features_not_liked",	false );
	addFeatureList( question );

	question = list->addQuestion( i18n( "Which features do you never use?" ), 	"features_never_used",	false );
	addFeatureList( question );

	question = list->addQuestion( i18n( "What is your favourite feature?" ), 	"favourite_feature",	true );
	addFeatureList( question );

	question = list->addQuestion( i18n( "Are there features you are missing?" ),	"features_missing",	true );
	question->addAnswer( i18n( "Yes, a lot! (please add comment below)"	),	"1/4_lots"		);
	question->addAnswer( i18n( "Some (please add comment below)"		),	"2/4_some"		);
	question->addAnswer( i18n( "None"					),	"3/4_none"		);
	question->addAnswer( i18n( "It has too many features already!"		),	"4/4_too_many_already"	);

	question = list->addQuestion( i18n( "How do you rate the stability of this program?" ),	"stability",	true, true );
	question->addAnswer( i18n( "Rock solid"					),	"1/5_rock_solid"		);
	question->addAnswer( i18n( "Good"					),	"2/5_good"		);
	question->addAnswer( i18n( "Average"					),	"3/5_average"		);
	question->addAnswer( i18n( "Poor"					),	"4/5_poor"		);
	question->addAnswer( i18n( "It keeps crashing all the time"		),	"5/5_keeps_crashing"	);

	question = list->addQuestion( i18n( "How do you rate the performance of this program?" ), "performance", true );
	question->addAnswer( i18n( "Great"					),	"1/5_great"		);
	question->addAnswer( i18n( "Good"					),	"2/5_good"		);
	question->addAnswer( i18n( "Average"					),	"3/5_average"		);
	question->addAnswer( i18n( "Poor"					),	"4/5_poor"		);
	question->addAnswer( i18n( "It's so slow it drives me nuts"		),	"5/5_drives_me_nuts"	);

	question = list->addQuestion( i18n( "What is your experience with computers in general?" ), "computer_experience", true );
	question->addAnswer( i18n( "Expert"					),	"1/5_expert"		);
	question->addAnswer( i18n( "Fair"					),	"2/5_fair"		);
	question->addAnswer( i18n( "Average"					),	"3/5_average"		);
	question->addAnswer( i18n( "Learning"					),	"4/5_learning"	);
	question->addAnswer( i18n( "Newbie"					),	"5/5_newbie"		);

	question = list->addQuestion( i18n( "What is your experience with Unix/Linux systems?" ), "unix_experience", true );
	question->addAnswer( i18n( "Expert"					),	"1/5_expert"		);
	question->addAnswer( i18n( "Fair"					),	"2/5_fair"		);
	question->addAnswer( i18n( "Average"					),	"3/5_average"		);
	question->addAnswer( i18n( "Learning"					),	"4/5_learning"	);
	question->addAnswer( i18n( "Newbie"					),	"5/5_newbie"		);

	question = list->addQuestion( i18n( "Did you have trouble figuring out how to work with this program?" ),
				      "learning_curve", true, true );
	question->addAnswer( i18n( "No problem"					),	"1/5_no_problem"	);
	question->addAnswer( i18n( "Some"					),	"2/5_some_problems"	);
	question->addAnswer( i18n( "I'm still learning"				),	"3/5_still_learing"	);
	question->addAnswer( i18n( "I didn't have a clue what to do at first"	),	"4/5_no_clue_at_first"	);
	question->addAnswer( i18n( "I still don't have a clue what to do"	),	"5/5_still_no_clue"	);

	question = list->addQuestion( i18n( "Where do you use this program most?" ),	"usage_where", 		true );
	question->addAnswer( i18n( "At work"					),	"at_work"		);
	question->addAnswer( i18n( "At home"					),	"at_home"		);
	question->addAnswer( i18n( "At university / school"			),	"university"		);

	question = list->addQuestion( i18n( "What is your primary role there?"	),	"primary_role",		true );
	question->addAnswer( i18n( "Home user"					),	"home_user"		);
	question->addAnswer( i18n( "Student"					),	"student"		);
	question->addAnswer( i18n( "Educational (teacher / professor)"		),	"educational"		);
	question->addAnswer( i18n( "Non-computer related work"			),	"non_computer"		);
	question->addAnswer( i18n( "Developer"					),	"developer"		);
	question->addAnswer( i18n( "System administrator"			),	"sysadmin"		);

	question = list->addQuestion( i18n( "Do you have any other roles there?" ),	"other_roles",		false );
	question->addAnswer( i18n( "Home user"					),	"home_user"		);
	question->addAnswer( i18n( "Student"					),	"student"		);
	question->addAnswer( i18n( "Educational (teacher / professor)"		),	"educational"		);
	question->addAnswer( i18n( "Non-computer related work"			),	"non_computer"		);
	question->addAnswer( i18n( "Developer"					),	"developer"		);
	question->addAnswer( i18n( "System administrator"			),	"sysadmin"		);

	question = list->addQuestion( i18n( "How did you get to know this program?" ),	"first_contact",	true );
	question->addAnswer( i18n( "In a menu on my machine"			),	"menu"			);
	question->addAnswer( i18n( "Somebody told me about it"			),	"told"			);
	question->addAnswer( i18n( "On the internet"				),	"internet"		);
	question->addAnswer( i18n( "Printed magazine / book"			),	"print_media"		);
	question->addAnswer( i18n( "Other (please add comment below)"		),	"other"			);

	list->addYesNoQuestion( i18n( "Did you ever get a KDirStat mail report telling you to clean up disk space?" ),
				"got_mail_report" );

	list->addYesNoQuestion( i18n( "Would you recommend this program to a friend?" ), "recommend", true );
    }

    if ( ! _feedbackDialog->isVisible() )
	_feedbackDialog->show();
}


void
KDirStatApp::addFeatureList( KFeedbackQuestion * question )
{
    question->addAnswer( i18n( "The directory tree display in general"			),	"tree_view"		);
    question->addAnswer( i18n( "Percentage bars as graphical display of relative sizes" ),	"percentage_bars" 	);
    question->addAnswer( i18n( "Files apart from directories in a separate <Files> item"),	"files_item"		);

    question->addAnswer( i18n( "Cleanup actions in general"				),	"cleanups_general"	);
    question->addAnswer( i18n( "Predefined cleanup actions"				),	"predefined_cleanups"	);
    question->addAnswer( i18n( "User defined cleanup actions"				),	"user_cleanups"		);
    question->addAnswer( i18n( "Cleanup action configuration"				),	"cleanup_config"	);

    question->addAnswer( i18n( "Different colors in percentage bars" 			),	"tree_colors"		);
    question->addAnswer( i18n( "Tree color configuration"				),	"tree_color_config"	);
    question->addAnswer( i18n( "Staying on one file system"				),	"stay_on_one_filesys"	);
    question->addAnswer( i18n( "The \"mail to owner\" facility"				),	"mail_to_owner"		);
    question->addAnswer( i18n( "This \"feedback mail\" facility"			),	"feedback"		);

    question->addAnswer( i18n( "Human readable sizes (kB, MB, ...)"			),	"human_readable_sizes"	);
    question->addAnswer( i18n( "All the numbers in the tree display"			),	"numeric_display"	);
    question->addAnswer( i18n( "Last change time of an entire directory tree"		),	"last_change_time"	);
    question->addAnswer( i18n( "The PacMan animation"					),	"pacman"		);
}



// EOF
