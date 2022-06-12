#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

void errorOpeningFile(char *filename);
void commandFile(char *filename, int option);
void handle_signal(int signal);
void handle_signalBatch(int signal);
void stringFilter(char *filename, char *extension, char *type, char *line);
void nonSupportedFiles(char *filename, char *extension, char *type, char *string);

#endif // FUNCTIONS_H_INCLUDED