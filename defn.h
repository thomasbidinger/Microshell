#include <stdio.h>

int expand (char *orig, char *new, int newsize);
int builtin(char ** args);
int processline (char *line, int outfd, int flags);

// extern char ** argvGlob;
// extern int argcGlob;
// extern char ** argvGlobOrig;
// extern int argcGlobOrig;


extern int argcGlob;
extern int argcGlobOrig;
extern char ** argvGlob;
extern char ** argvGlobOrig;

extern int exitValue;

extern int shiftAmount;

extern FILE * INFILE;
