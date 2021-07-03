/* CS 352 -- Micro Shell!  
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001 
 *   Modified January 6, 2003
 *   Modified January 8, 2017
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defn.h"
#include <signal.h>


/* Constants */ 

#define LINELEN 1024
#define WAIT 1
#define NOWAIT 0

/* Prototypes */
void sighandler(int);
char ** arg_parse(char *line, int *argcptr);

int processline (char *line, int infd, int outfd, int flags);

/* Shell main */
int argcGlob;
int argcGlobOrig;
char ** argvGlob;
char ** argvGlobOrig;
int shiftAmount = 0;
FILE * INFILE;
int exitValue = 0;

int main (int argc, char * argv[])
{
    signal(SIGINT, sighandler);

    char   buffer [LINELEN];
    int    len;

    //set input file, if/if not specified
    INFILE = stdin;
    if(argc >= 2) {
      INFILE = fopen(argv[1], "r");
      if(INFILE == 0) {
        fprintf(stderr, "file doesn't exist\n");
        exit(127);
      }
    }

    //set global variables
    argcGlob = argc;
    argcGlobOrig = argc;
    argvGlob = argv;
    argvGlobOrig = argv;

    while (1) {

            /* prompt and get line */
      if(INFILE == stdin) {
        fprintf (stderr, "%%");
      }
      
      if (fgets (buffer, LINELEN, INFILE) != buffer){
        break;
      }
        

            /* Get rid of \n at end of buffer. */
      len = strlen(buffer);
      
      if (buffer[len-1] == '\n')
          buffer[len-1] = 0;

      //check if first line is a comment
      if(buffer[0] != '#'){
        /* Run it ... */
        processline (buffer, 0, 1, WAIT);
      }
    
    }

      if (!feof(INFILE))
          perror ("read");

      return 0;		/* Also known as exit (0); */
}

// parse number of arguments, return array with elements
char ** arg_parse (char *line, int *argcptr)
{
    int argc = 0;
    int i = 0;
    // parses number of arguments
    while(line[i] != '\0') {
      if(line[i] == ' ') {
        /*skip*/
        i++;
      }else {
        argc++;
        while(line[i] != ' ' && line[i] != '\0') {
          if(line[i] == '"') {
            i++;
            while(line[i] != '"') {
              if(line[i] == '\0') {
                printf("Standard error");
                return NULL;
              }
              i++;
            }
            i++;
          }else {
            i++;
          }
        }
      }
    }

    char **args;
    args = (char**)malloc((argc + 1) * sizeof(char *));
    if(args == NULL) {
      fprintf(stderr, "malloc failed");
    }
    args[argc] = NULL;

    // assign pointers
    i = 0;
    int j = 0;
    while(line[i] != '\0') {
      if(line[i] == ' ') {
        i++;
      }else { //if not in quotes
        args[j] = &line[i];
        j++;
        while(line[i] != ' ' && line[i] != '\0') {
          if(line[i] == '"') {
            i++;
            while(line[i] != '"') {
              i++;
            }
            i++;
          }else {
            i++;
          }
        }
        if(line[i] != '\0') {
          line[i] = '\0';
          i++;
        }
      }
    }

    *argcptr = argc;
    //remove quotes
    i = 0;
    while(args[i] != NULL && argc != 0) {
      char * src = args[i];
      char * dst = args[i];
      while(*src != 0) {
        if(*src != '"') {
          *dst = *src;
          dst++;
        }
        src++;
      }
      * dst = '\0';
      argc--;
      i++;
    }
    

    return args;  
}


int processline (char *line, int infd, int outfd, int flags)
{
    pid_t  cpid;
    int    status;
    
    int newSize = 1024;
    char newLine [newSize];
    memset(newLine, 0, newSize);
    if(expand(line, newLine, newSize) < 0) {
      return -1;
    }

    int argcount = 0;
    char ** args = arg_parse(newLine, &argcount);

    //do check to see if the command is a built-in
    if(builtin(args, argcount) == 1) {
      return -1;
    }

    /* Start a new process to do the job. */
    cpid = fork();
    if (cpid < 0) {
      /* Fork wasn't successful */
      exitValue = 1;
      perror ("fork");
    }
    
    /* Check for who we are! */
    if (cpid == 0) {
      /* We are the child! */
      // execlp (line, line, (char *)0);

      //set outfd, infd
      if(outfd != 1) {
        dup2(outfd, 1);
      }
      if(infd != 0) {
        dup2(infd, 0);
      }

      execvp (args[0], args);
      /* execlp reurned, wasn't successful */
      perror ("exec");
      fclose(INFILE);  // avoid a linux stdio bug
      free(args);
      exit (127);
    }

    /* Have the parent wait for child to complete */
    if(flags == WAIT) {
      if (wait (&status) < 0) {
        /* Wait wasn't successful */
        perror ("wait");
      }else {
        if(WIFEXITED(status)) {
          exitValue = WEXITSTATUS(status);
        }else if(WIFSIGNALED(status)) {
          exitValue = WTERMSIG(status);
        }
      }
      return -1;
    }else {
      return cpid;
    }

    
}

void sighandler(int signum) {
  if(signum == 5) {
    printf("core dumped");
  }
  //psignal(WTERMSIG(signum), "signal");
      exitValue = 128 + signum;
}




