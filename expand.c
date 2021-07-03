#include "defn.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/wait.h>

/* Constants */ 

#define LINELEN 1024
#define WAIT 1
#define NOWAIT 0

char  buffer [LINELEN];
void getpipe(char *str, char *i, int fd);


void getpipe(char *str, char *i, int fd) {
  char *pIndex = strchr(i, '|');
  if (pIndex != NULL) {
    *i = '\0';
    i++;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
      fprintf(stderr, "Pipe failed.");
      return;
    }
    processline(str, fd, pipefd[1], NOWAIT);
    getpipe(i, pIndex, pipefd[0]);
  }
  else {
    *i = '\0';
    i++;
    processline(str, fd, 1, WAIT);
  }
}

//int expand (char *orig , char *new, int newsize);
int expand (char *orig , char *new, int newsize) {
    int i = 0;
    int j = 0;
    int k = 0;
    char * temp = malloc(sizeof(char) * 24);
    //iterates chars in origin
    while(orig[i] != '\0'){
        if(i >= newsize) {
            fprintf(stderr, "expansion overflow");
            return -1;
        }
        new[j] = orig[i];
        j++;
        i++;
        //goes over environment variable
        if(orig[i] == '$' && orig[i+1] == '{'){
            k = 0;
            i = i + 2;

            while(orig[i] != '}'){
                if(orig[i] == 0) {
                    fprintf(stderr, "missing }");
                    return -1;
                }
                temp[k] = orig[i];
                i++;
                k++;
            }
            temp[i] = 0;
            i++;

            //insert environment variable
            k = 0;
            char * variable = NULL;
            variable = getenv(temp);
            //check if env variable was real
            if(variable) {
                memset(temp, 0, sizeof(char) * 24);
                while(variable[k] != 0) {
                    new[j] = variable[k];
                    j++;
                    k++;
                }
            }
        }

        //handles comments
        if(orig[i-1] != '$' && orig[i] == '#') {
            new[j] = '\0';
            j++;
            break;
        }

        //handles $# pattern
        if(orig[i] == '$' && orig[i+1] == '#') {
            int num = argcGlob - 1 - shiftAmount;
            char snum[10];

            snprintf(snum, 10, "%i", num);

            int x = 0;
            while(snum[x] != '\0') {
                new[j] = snum[x];
                x++;
                j++;
            }

            i = i + 2;
        }

        //handles $n pattern
        if(orig[i] == '$' && isdigit(orig[i+1])) {
            //get number into n
            int k = i + 1;
            char numArr [10] = "";
            int x = 0;
            while(isdigit(orig[k])) {
                numArr[x] = orig[k];
                k++;
                x++;
            }
            int n = atoi(numArr);

            //replace $n with argument
            if(argcGlob == 1) {
                if(n == 0) {
                    char * argument = argvGlob[n];  
                    int length = (int)strlen(argument);
                
                    for (int p = 0; p < length; p++) {
                        new[j] = argument[p];
                        j++;
                    }
                }
            }else if(n+1 < argcGlob) {
                char * argument;
                if(argvGlob[n+1+shiftAmount]) {
                  argument = argvGlob[n+1+shiftAmount];   
                }else {
                    argument = "";
                }
                 
                int length = (int)strlen(argument);
            
                for (int p = 0; p < length; p++) {
                    new[j] = argument[p];
                    j++;
                }
            }
            
            

            // while(orig[i] != ' ' && orig[i] != 0) {
            //     i++;
            // }
            i++;
            while(isdigit(orig[i]) && orig[i] != 0) {
                i++;
            }



        }

        //checks for PID keyword and replaces
        if(orig[i] == '$' && orig[i+1] == '$') {
            k = 0;
            i = i + 2;
            char PID [100];
            snprintf(PID, 100, "%d", getpid());
            while(PID[k] != 0) {
                new[j] = PID[k];
                j++;
                k++;
            }
        }

        //wildcard expansion
        if(orig[i] == '*' && orig[i - 1] != '\\') {
            DIR *dr;
            struct dirent *dir;
            dr = opendir(".");
            char fileName[255];

            if(dr) {
                //when * has trailing whitespace
                if((orig[i+1] == ' ' || orig[i+1] == '\0') && orig[i-1] == ' ') {
                    while((dir = readdir(dr)) != NULL) {
                        snprintf(fileName, 256, "%s", dir->d_name);
                        fileName[254] = '\0';
                        
                        if(fileName[0] != '.') {
                            int x = 0;
                            while(fileName[x] != '\0') {
                                new[j] = fileName[x];
                                j++;
                                x++;
                            }
                            new[j] = ' ';
                            j++;
                        }
                    } 
                //when * has trailing non-whitespace
                }else if((orig[i+1] != ' ' || orig[i+1] != '\0') && orig[i-1] == ' ') {
                    char matcher[255];
                    int x = i + 1;
                    int k = 0;

                    //get wildcard expression
                    while(orig[x] != ' ' && orig[x] != '\0') {
                        matcher[k] = orig[x];
                        k++;
                        x++;
                    }

                    if(strstr(matcher, "/") != NULL) {
                        fprintf(stderr, "No '/'! Cannot process current line.");
                        break;
                    }

                    //get fileName
                    while((dir = readdir(dr)) != NULL) {
                        snprintf(fileName, 256, "%s", dir->d_name);
                        fileName[254] = '\0';
                        
                        if(fileName[0] != '.' && strstr(fileName, matcher) != NULL) {
                            int x = 0;
                            while(fileName[x] != '\0') {
                                new[j] = fileName[x];
                                j++;
                                x++;
                            }
                            new[j] = ' ';
                            j++;
                        }
                    }
                    
                }else {
                    int x = i;
                    while(orig[x] != ' ' && orig[x] != 0) {
                        new[j] = orig[x];
                        j++;
                        x++;
                    }   
                }
                
                closedir(dr);

                
            }
            
            //move orig index i past what was expanded
            i++;
            while(orig[i] != ' ' && orig[i] != 0) {
                i++;
            }     
        }

        //escape character before wildcard
        if(orig[i] == '\\' && orig[i + 1] == '*') {
            new[j] = '*';
            j++;
            i = i + 2;
        }

        //expansion for $? pattern
        if(orig[i] == '$' && orig[i + 1] == '?') {
            char exitStr[10];
            sprintf(exitStr, "%d", exitValue);
            
            int x = 0;
            while(exitStr[x] != '\0') {
                new[j] = exitStr[x];
                j++;
                x++;
            }
            
            i = i + 2;
        }

        if(orig[i] == '$' && orig[i + 1] == '(') {
            i = i + 2;
            int start = i;

            int paren = 0;
            while(orig[i] != ')' || paren > 0) {
                if(orig[i] == '\0') {
                    fprintf(stderr, "Bad syntax, no matching parenthesis.");
                    return -1;
                }
                if(orig[i] == '(') {
                    paren++;
                }
                if(orig[i] == ')') {
                    paren--;
                }
                i++;
            }

            char copy = orig[i];
            orig[i] = '\0';

            //piping and forking
            int pipefd[2];
            //pid_t cpid;
            char substring[20000];

            //open pipes, check for errors
            if(pipe(pipefd) == -1) {
                fprintf(stderr, "Pipe Failed");
                return -1;
            }

            processline(&orig[start], 0, pipefd[1], WAIT);
            orig[i] = copy;
            i++;
            close(pipefd[1]);
            read(pipefd[0], substring, 20000);
            close(pipefd[0]);
            int x = 0;
            while(substring[x] != '\0' && substring[x] != '\n') {
                if(substring[x] != '\n') {
                    new[j] = substring[x];
                    j++;
                }
                x++;
            }

        }

        char *pIndex = strchr(new, '|');
        if (pIndex != NULL) {
            getpipe(new, pIndex, 0);
        }

        
    }
    return 1;    
}

