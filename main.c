/*
	Tiago Júlio Santos: 2201755
	Rodrigo Filipe Capitão: 2201741
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

#include "debug.h"
#include "args.h"
#include "memory.h"
#include "functions.h"
#include "constants.h"
#include "structs.h"

int main(int argc, char *argv[])
{
	/* Disable warnings */

	(void)argc;
	(void)argv;

	/* Main code */

	struct gengetopt_args_info args;

	/* Variables */

	FILE *fd, *fs;
	char filename[MAX_STRING], extension[MAX_STRING], type[MAX_STRING], line[MAX_STRING], countOK = 0, countMismatch = 0, countNonSupported=0, countErrors = 0, countLines=0;
	int option = 0;

	printf("\n ");

	if (cmdline_parser(argc, argv, &args) != 0) {
		exit(1);
	}

	if (args.file_given == 0 && args.batch_given == 0 && args.dir_given == 0) {

		printf("Error: No arguments given\n");
	}

	printf("\b");

	/* Option File */

	if (args.file_given) {

		/* Cicles through all files */

		for (unsigned int i = 0; i < args.file_given; i++) {

			printf(" ");

			fd = fopen(args.file_arg[i], "rb");

			if (fd == NULL) {

				errorOpeningFile(args.file_arg[i]);
				fprintf(stderr, " ERROR: cannot open file <%s> -- %s\n", filename, strerror(errno));
    			printf("\b");
				continue;
			}

			/* Calls the commandFile function to analize the file */

			commandFile(args.file_arg[i], option);

			fclose(fd);

			/* Calls the stringFilter function to analize the results */

			stringFilter(args.file_arg[i], extension, type, line);

			/* Checks if the extension and type match */

			if ((strcmp(extension, type) != 0) && ((strcmp(type, "pdf") == 0 || strcmp(type, "gif") == 0 || strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "mp4") == 0 || strcmp(type, "zip") == 0 || strcmp(type, "html") == 0))) {

				printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", args.file_arg[i], extension, type);
			
			} else {

				if ((strcmp(extension, type) == 0) && (strcmp(extension, "pdf") == 0 || strcmp(extension, "gif") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "png") == 0 || strcmp(extension, "mp4") == 0 || strcmp(extension, "zip") == 0 || strcmp(extension, "html") == 0) && (strcmp(type, "pdf") == 0 || strcmp(type, "gif") == 0 || strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "mp4") == 0 || strcmp(type, "zip") == 0 || strcmp(type, "html") == 0)) {

					printf("[OK] '%s': extension '%s' matches file type '%s'\n", args.file_arg[i], extension, type);
				
				} else {
					
					nonSupportedFiles(args.file_arg[i], extension, type, line);
				}
			}
		}
	}

	/* Option Batch */

	if (args.batch_given)
	{
		typeFilename arrayFilename[MAX_FILES];
		int i=0, returnedValue; 
		option = 1;

		fd = fopen(args.batch_arg, "r");

		if (fd == NULL) {

			errorOpeningFile(args.batch_arg);
			printf("\n");
			exit(1);
		}

		printf(" [INFO] analyzing files listed in ‘%s’\n\n", args.batch_arg);

		/* Stores the filenames from the batch file in an array */

		while ((returnedValue = fscanf(fd, "%s\n", arrayFilename[i].filename)) > 0) {

			i++;
			countLines++;
		}

		/* Cicles through all files */

		for (i = 0; i < countLines; i++) {

			printf(" ");

			fs = fopen(arrayFilename[i].filename, "rb");

			if (fs == NULL) {

				errorOpeningFile(arrayFilename[i].filename);
				countErrors++;
				continue;
			}

			/*Calls the commandFile function to analize the file */

			commandFile(arrayFilename[i].filename, option);

			fclose(fs);

			/*Calls the stringFilter function to analize the results */

			stringFilter(arrayFilename[i].filename, extension, type, line);

			/* Checks if the extension and type match */

			if ((strcmp(extension, type) != 0) && ((strcmp(type, "pdf") == 0 || strcmp(type, "gif") == 0 || strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "mp4") == 0 || strcmp(type, "zip") == 0 || strcmp(type, "html") == 0))) {

				printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", arrayFilename[i].filename, extension, type);
				countMismatch++;
			
			} else {

				if ((strcmp(extension, type) == 0) && (strcmp(extension, "pdf") == 0 || strcmp(extension, "gif") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "png") == 0 || strcmp(extension, "mp4") == 0 || strcmp(extension, "zip") == 0 || strcmp(extension, "html") == 0) && (strcmp(type, "pdf") == 0 || strcmp(type, "gif") == 0 || strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "mp4") == 0 || strcmp(type, "zip") == 0 || strcmp(type, "html") == 0)) {
					
					printf("[OK] '%s': extension '%s' matches file type '%s'\n", arrayFilename[i].filename, extension, type);
					countOK++;
				
				} else {

					nonSupportedFiles(arrayFilename[i].filename, extension, type, line);
					countNonSupported++;
				}
			}
		}

		/* Prints the number of files analyzed, number of OKs and mismatches, and number of errors */

		countLines-=countNonSupported;
		printf("\n [SUMMARY] Files Analyzed: %d; Files OK: %d; Mismatch: %d; Errors: %d\n", countLines, countOK, countMismatch, countErrors);
	}

	/* Option Directory */

	if (args.dir_given) {

		struct dirent *de;
		DIR *dr;
		typeFilename arrayFilename[MAX_FILES];
		int i=0, countFiles=0;

		dr = opendir(args.dir_arg);

		if (dr == NULL) { /* opendir returns NULL if not possible to open directory */

			fprintf(stderr, "ERROR: cannot open dir '%s' -- %s\n", args.dir_arg, strerror(errno));
			return 0;
		}

		printf(" [INFO] analyzing files of directory ‘%s’\n\n", args.dir_arg);

		/* Analizes content in the directory */

		while ((de = readdir(dr))) {

			if (!strcmp(de->d_name, "."))
				continue;
			
			if (!strcmp(de->d_name, ".."))
				continue;

			/* Checks if there is a ".", if there isn't it means it's in a sub-directory and will be ignored */

			if (strstr(de->d_name, ".")) {
				
				strcpy(arrayFilename[i].filename, de->d_name);	/* Stores the filename in an array */
				countFiles++;
				i++;
			}
		}

		closedir(dr);

		/* Cicles through all files */

		for (i = 0; i < countFiles; i++) {

			printf(" ");

			strcpy(filename,"");

			fs = fopen(arrayFilename[i].filename, "rb");

			if (fs == NULL) {

				errorOpeningFile(arrayFilename[i].filename);
				countErrors++;
				continue;
			}

			/* Calls the commandFile function to analize the file */

			commandFile(arrayFilename[i].filename,option);

			fclose(fs);

			/* Calls the stringFilter function to analize the results */
			
			stringFilter(arrayFilename[i].filename, extension, type, line);
			
			/* Copy directory to filename on output */

			strcat(filename,args.dir_arg);
			strcat(filename, "/");
			strcat(filename,arrayFilename[i].filename);

			/* Checks if the extension and type match */

			if ((strcmp(extension, type) != 0) && ((strcmp(type, "pdf") == 0 || strcmp(type, "gif") == 0 || strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "mp4") == 0 || strcmp(type, "zip") == 0 || strcmp(type, "html") == 0))) {

				printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", filename, extension, type);
				countMismatch++;
			
			} else {

				if ((strcmp(extension, type) == 0) && (strcmp(extension, "pdf") == 0 || strcmp(extension, "gif") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "png") == 0 || strcmp(extension, "mp4") == 0 || strcmp(extension, "zip") == 0 || strcmp(extension, "html") == 0) && (strcmp(type, "pdf") == 0 || strcmp(type, "gif") == 0 || strcmp(type, "jpg") == 0 || strcmp(type, "png") == 0 || strcmp(type, "mp4") == 0 || strcmp(type, "zip") == 0 || strcmp(type, "html") == 0)) {

					printf("[OK] '%s': extension '%s' matches file type '%s'\n", filename, extension, type);
					countOK++;
				
				} else {

					nonSupportedFiles(filename, extension, type, line);
					countNonSupported++;
				}
			}
		}

		/* Prints the number of files analyzed, number of OKs and mismatches, and number of errors */

		countFiles-=countNonSupported;
		printf("\n [SUMMARY] Files Analyzed: %d; Files OK: %d; Mismatch: %d; Errors: %d\n", countFiles, countOK, countMismatch, countErrors);
	}

	/* Deletes the file "result.txt" and frees variable args */

	unlink("result.txt");
	printf("\n");
	cmdline_parser_free(&args);

	return 0;
}