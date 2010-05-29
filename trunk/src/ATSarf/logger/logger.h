#ifndef _LOGGER_
#define _LOGGER_

#include <QTextStream>
#include <bitset>

extern QTextStream out;
extern QTextStream in;
extern QTextStream displayed_error;
extern QTextStream hadith_out;

extern bool KEEP_OLD;
extern bool warn_about_automatic_insertion;
extern bool display_errors;
extern bool display_warnings;

#define error \
	if (display_errors) displayed_error << "ERROR! "
			//... pay attention to put such statements between {..} in if-else structures, otherwise next else would be to this if and not as intended
#define warning \
	if (display_errors && display_warnings) displayed_error << "WARNING! "
		//...

#endif /*_LOGGER */
