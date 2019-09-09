#include "data.h"

#ifndef PROTOTYPES
#define PROTOTYPES	
	//Type def for struct dirent renaming
	typedef struct dirent dirent;

	//Handles the reading, parsing, sorting and outputting of the sorted data to the file
	int handle_sort(Mode mode, char* sort_cat, char* input_dir, char* output_dir);

	//Gets reads and parses the execution arguments
	int parse_exec_args(int argc, char **argv, Mode* mode, char** sort_cat, char** input_dir, char** output_dir);
#endif
