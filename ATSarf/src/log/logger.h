#ifndef _LOGGER_
#define _LOGGER_

#include <sarf.h>
#include <QTextStream>

//#include <QDebug>

//extern QTextStream out;
//extern QTextStream in;
//extern QTextStream displayed_error;
extern Sarf * theSarf;
extern QTextStream hadith_out;

extern bool KEEP_OLD;
extern bool warn_about_automatic_insertion;
extern bool display_errors;
extern bool display_warnings;

#define error \
        if (theSarf!= NULL && display_errors)  theSarf->displayed_error << "ERROR! "
			//... pay attention to put such statements between {..} in if-else structures, otherwise next else would be to this if and not as intended
#define warning \
        if (theSarf!= NULL && display_errors && display_warnings) theSarf->displayed_error << "WARNING! "
		//...

#endif /*_LOGGER */
