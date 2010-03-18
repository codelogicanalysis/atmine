#ifndef _LOGGER_
#define _LOGGER_

#include <QTextStream>
#include <bitset>

QTextStream out;
QTextStream in;
QTextStream displayed_error;

bool KEEP_OLD=true;
bool warn_about_automatic_insertion =false;
bool display_errors=true;
bool display_warnings=true;

#define error \
	if (display_errors) displayed_error << "ERROR! "
			//... pay attention to put such statements between {..} in if-else structures, otherwise next else would be to this if and not as intended
#define warning \
	if (display_errors && display_warnings) displayed_error << "WARNING! "
		//...

#endif /*_LOGGER */
