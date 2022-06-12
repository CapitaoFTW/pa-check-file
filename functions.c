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
#include <time.h>

#include "debug.h"
#include "args.h"
#include "memory.h"
#include "constants.h"
#include "functions.h"

/* Function that prints the ERROR */

void errorOpeningFile(char *filename) {

    fprintf(stderr, " ERROR: cannot open file <%s> -- %s\n", filename, strerror(errno));
    printf("\b");
}

/* Function to analize the files */

void commandFile(char *filename, int option) {

    int fdes, erro;
    pid_t pid;
    struct sigaction act;

    switch (pid = fork()) {
    
        case -1: // error

            fprintf(stderr, "ERROR: %s", strerror(errno));
            exit(1);

        break;

        case 0: // child process

            /* Result of command "file --extension" is sent to file "result.txt" */

            fdes = open("result.txt", O_RDWR | O_CREAT, 0777);
            dup2(fdes, 1);
            close(fdes);

            erro = execlp("file", filename, "--mime", filename, NULL);

            if (erro == -1) {

                fprintf(stderr, "ERROR: cannot execute command 'file' -- %s\n", strerror(errno));
            }

        break;

        default: // parent process

            wait(NULL);
            sleep(1);

            /* Signal response routine */

            if (option == 1) {

                act.sa_handler = handle_signalBatch;
            
            } else {

                act.sa_handler = handle_signal;
            }

            sigemptyset(&act.sa_mask);

            act.sa_flags = SA_RESTART;

            if (sigaction(SIGQUIT, &act, NULL) != 0) {

                fprintf(stderr, "ERROR: sigaction() failed %s\n", strerror(errno));
            }

            if (sigaction(SIGUSR1, &act, NULL) != 0) {

                fprintf(stderr, "ERROR: sigaction() failed %s\n", strerror(errno));
            }

        break;
    }
}

/* Function to treat signals in File and Directory Options */

void handle_signal(int signal) {

    /* Copies global variable errno */

    int aux = errno;

    /* Signal SIGQUIT */

    if (signal == SIGQUIT){

        printf("Captured SIGQUIT signal (sent by PID: %d). Use SIGINT to terminate application.\n", getpid());
        kill(getpid(), SIGQUIT);
    }
    
    /* Restores errno value */

    errno = aux;
}

/* Function to treat signals in Batch Option */

void handle_signalBatch(int signal) {

	time_t T = time(NULL);
    struct tm tm = *localtime(&T);
    
    /* Copies global variable errno */

    int aux = errno;

    /* Signal SIGUSR */

    if (signal == SIGUSR1) {

        printf("Processing started: %04d.%02d.%02d_%02dh%02d:%02d\n",tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    /* Signal SIGQUIT */

    if (signal == SIGQUIT) {

        printf("Captured SIGQUIT signal (sent by PID: %d). Use SIGINT to terminate application.\n", getpid());
        kill(getpid(), SIGQUIT);
    }
    
    /* Restores errno value */

    errno = aux;
}

/* Function to compare extension and file type */

void stringFilter(char *filename, char *extension, char *type, char *line) {

    FILE *fs;
    char file[MAX_STRING], lineAux[MAX_STRING];

    /* This opens "result.txt" and extracts the extension and the file type, then it compares both to see if they match or not */

    fs = fopen("result.txt", "r");

    if (fs == NULL) {

        errorOpeningFile("result.txt");
        exit(1);
    }

    fgets(line, MAX_STRING, fs);
    fclose(fs);

    strcpy(file, filename);
    strtok(file, " .");
    strcpy(extension, strtok(NULL, " ."));

    strcpy(lineAux, line);

    if (strstr(file, "/") != NULL) { /* For those cases where the file is located in another directory */

        strtok(lineAux, ": ;");
        strcpy(lineAux, strtok(NULL, ": ;"));
        strtok(lineAux,"/");
        strcpy(type, strtok(NULL, "/"));
        
    } else {

        strtok(lineAux, "/;"); /* normal cases (supported files and same location as the program) */
        strcpy(type, strtok(NULL, "/;"));
    }

    if (strcmp(type, "jpeg") == 0)
    strcpy(type, "jpg");

}


/* Function to print special message if file isn't supported */

void nonSupportedFiles(char *filename, char *extension, char *type, char *string) {

    strcpy(type, extension);
    strtok(string, " :;");
    type = strtok(NULL, " :;");
    printf("[INFO] '%s': type '%s' is not supported by checkFile\n", filename, type);
}