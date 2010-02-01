#include "k4dirstat.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KDE/KLocale>
#include <KDE/KUrl>

static const char description[] =
    I18N_NOOP("A KDE 4 Application");

static const char version[] = "%{VERSION}";

int main(int argc, char **argv)
{
    KAboutData about("k4dirstat", 0, ki18n("k4dirstat"), version, ki18n(description),
                     KAboutData::License_GPL, ki18n("(C) 2007 %{AUTHOR}"), KLocalizedString(), 0, "%{EMAIL}");
    about.addAuthor( ki18n("%{AUTHOR}"), KLocalizedString(), "%{EMAIL}" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("+[Dir/URL]", ki18n( "Directory or URL to open" ));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

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
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();      
        if (args->count() == 0)
        {
            kdirstat->fileAskOpenDir();
        }
        else
        {       
            // Process command line arguments as URLs or paths to scan

            KUrl url = args->url( 0 );
            // kdDebug() << "Opening " << url.url() << endl;
            kdirstat->openURL( url );
        }
        args->clear();
    }

    return app.exec();
}
