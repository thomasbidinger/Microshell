#include "defn.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include </home/phil/public/csci347/strmode.c>
#include <sys/stat.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

//built-in functions
void exit_ex(char ** args){
    if(args[1] == NULL) {
        exitValue = 0;
        exit(0);
    }else {
        int temp;
        temp = atoi(args[1]);
        exitValue = temp;
        exit(temp);
    }
}

void envset_ex(char ** args){
    setenv(args[1], args[2], 1);
}

void envunset_ex(char ** args) {
    unsetenv(args[1]);
}

void cd_ex(char ** args) {
    if(args[1] != NULL) {
        if(chdir(args[1]) < 0) {
            exitValue = 1;
            fprintf(stderr, "not a directory");
        }
    }else {
        if(chdir(getenv("HOME")) < 0) {
            exitValue = 1;
            fprintf(stderr, "not a directory");
        }
    }
}

void shift_ex(char ** args) {
    int n = 1;
    if(args[1] != NULL) {
      n = atoi(args[1]);  
    }
    //check if n is valid
    if(n >= (argcGlob-1)) {
        exitValue = 1;
        fprintf(stderr, "shift cannot be done with input");
    }
    
    shiftAmount = n;
}

void unshift_ex(char ** args) {
    if(args[1] == NULL) {
        shiftAmount = 0;        
    }else {
        if(atoi(args[1]) <= shiftAmount) {
            shiftAmount = shiftAmount - atoi(args[1]);
        }else {
            exitValue = 1;
            fprintf(stderr, "shift cannot be done with input");
        }
        
    }
}

void sstat_ex(char ** args) {
    int i = 2;
    char * fileName;

    struct passwd *pwd;
    struct group *grp;
    struct stat sb;
    char permissions[256];

    while(args[i] != NULL) {
        fileName = args[i];
        i++;

        if (stat(fileName, &sb) == -1) {
            exitValue = 1;
            perror("stat");
            break;
        }

        //print file name
        printf("%s ", fileName);
    
        //print user name
        pwd = getpwuid(sb.st_uid);
        if (pwd == NULL) {
            printf("%i ", sb.st_uid);
        }else {
            printf("User Name: %s ", pwd->pw_name);
        }
        

        //print group name
        grp = getgrgid(sb.st_gid);
        if (grp == NULL) {
            printf("%i ", sb.st_gid);
        }else {
            printf("Group Name: %s ", grp->gr_name);
        }
        
        //print permission list
        strmode(sb.st_mode, permissions);
        printf("Permissions: %s ", permissions);

        //print number of links
        printf("Links: %li ", sb.st_nlink);

        //print size in bytes
        printf("Size: %li ", sb.st_size);

        //print modification time
        char time[10];
        strftime(time, 20, "%d-%m-%y %H:%M:%S", localtime(&(sb.st_ctime)));
        printf("Modification time: %s ", time);

        printf("\n");
    }
}

//in-order function pointers
void (*fun_ptr_arr[])(char **) = {exit_ex, envset_ex, envunset_ex, cd_ex, shift_ex, unshift_ex, sstat_ex};

int builtin(char ** args, int argc) {
    //set array of built-in commands
    const char *builtInCommands[8];
    builtInCommands[0] = "exit";
    builtInCommands[1] = "envset";
    builtInCommands[2] = "envunset";
    builtInCommands[3] = "cd";
    builtInCommands[4] = "shift";
    builtInCommands[5] = "unshift";
    builtInCommands[6] = "sstat";
    builtInCommands[7] = 0;

    //check if input is nothing
    if(argc == 0) {
        return 1;
    }

    //loop thru built-in commands, call matching function
    int i = 0;
    int flag = 0;
    while(builtInCommands[i] != 0 && flag == 0) {
        if(strcmp(args[0], builtInCommands[i]) == 0) {
            exitValue = 0;
            (*fun_ptr_arr[i])(args);
            flag = 1;
        }
        i++;
    }
    
    if(flag == 0) {
        return -1;
    }else {
        return 1;
    }
}
