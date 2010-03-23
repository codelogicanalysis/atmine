#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

/*
 * Executable to augment disctionary
 */
int main(int , char** ) {//change to err and output of command line or output file

	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
	if (insert_buckwalter()<0)
	return -1;
	if (insert_propernames()<0)
		return -1;
	if (insert_placenames()<0)
		return -1;
	return (EXIT_SUCCESS);
}

