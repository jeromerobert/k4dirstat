/*
 *   File name:	main.cpp
 *   Summary:	Main program for KDirStat
 *   License:	GPL - See file COPYING for details.
 *
 *   Author:	Stefan Hundhammer <kdirstat@gmx.de>
 *		Joshua Hodosh <kdirstat@grumpypenguin.org>
 *		Parts auto-generated by KDevelop
 *
 *   Updated:	2010-03-03
 */



#include "k4dirstat.h"
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <KDE/KLocale>
#include <KDE/KUrl>

static const char description[] =
    I18N_NOOP("k4dirstat - Directory statistics.\n"
		"\n"
		"Shows where all your disk space has gone\n"
		"and helps you clean it up.");

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)
static const char version[] = EXPAND(K4DIRSTAT_VERSION);

int main(int argc, char **argv)
{
    KAboutData about("k4dirstat", i18n("k4dirstat"), version, i18n(description),
                     KAboutLicense::GPL,
                     "\u00A9 2015 J\u00E9r\u00F4me Robert, \u00A9 2010 Joshua Hodosh, \u00A9 1999-2008 Stefan Hundhammer",
                     "", "https://bitbucket.org/jeromerobert/k4dirstat",
                     "https://bitbucket.org/jeromerobert/k4dirstat/issues");

    about.addAuthor("J\u00E9r\u00F4me Robert", i18n("KF5 Port, current maintainer." ), "",
		      "https://bitbucket.org/jeromerobert/k4dirstat" );
    about.addAuthor("Stefan Hundhammer",
                      i18n("Original kdirstat author." ), "kdirstat@gmx.de",
		      "http://kdirstat.sourceforge.net/" );
    about.addAuthor("Joshua Hodosh", i18n("Port to KDE4"),
                     "kdirstat@grumpypenguin.org" );

    about.addCredit( i18n("SequoiaView Team"),
                      i18n( "for showing just how useful treemaps really can be." ),
		      0,	// e-mail
		      "http://www.win.tue.nl/sequoiaview" );

    about.addCredit( i18n("Jarke J. van Wijk, Huub van de Wetering, Mark Bruls"),
                      i18n( "for their papers about treemaps." ),
		      "vanwijk@win.tue.nl",
		      "http://www.win.tue.nl/~vanwijk/" );

    about.addCredit( i18n("Ben Shneiderman"),
                      i18n( "for his ingenious idea of treemaps -\n"
				 "a truly intuitive way of visualizing tree contents." ),
		      "",	// E-Mail
		      "http://www.cs.umd.edu/hcil/treemaps/" );
    KAboutData::setApplicationData(about);
    QApplication app(argc, argv);
    app.setApplicationName("k4dirstat");
    app.setApplicationVersion(version);
    app.setApplicationDisplayName(i18n("k4dirstat"));
    app.setWindowIcon(QIcon::fromTheme("k4dirstat"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("+[Dir/URL]", "Directory or URL to open");
    parser.process(app);
    k4dirstat *kdirstat = new k4dirstat;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(k4dirstat);
    }
    else
    {
        kdirstat->show();
        // no session.. just start up normally
        QStringList args = parser.positionalArguments();
        if (args.isEmpty())
        {
            kdirstat->fileAskOpenDir();
        }
        else
        {       
            // Process command line arguments as URLs or paths to scan
            kdirstat->openURL(args[0]);
        }
    }

    return app.exec();
}
