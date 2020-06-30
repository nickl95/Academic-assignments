/*
 * Nicholas LaFramboise
 * fj1000 - CSC 4420
 * Takehome Lab #4
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50
#define HISTORY_LENGTH 10

static char buffer[BUFFER_SIZE];
static char hist_cmd[HISTORY_LENGTH][MAX_LINE]; // the array to record history 
int head; // history cmds list head: the index of next command
int cmd_counter; // the number of commands in total
int isSIGINT = 0; // if current command is <ctrl><c>
/* the signal handler function */
void handle_SIGINT() {
        isSIGINT = 1;
        write(STDOUT_FILENO,buffer,strlen(buffer));
        if(cmd_counter == 0)
                return;
        int idx = -1; // the command index in the hist_cmd[] array
        int id = -1; // the command id, starting from 1
        if(cmd_counter<HISTORY_LENGTH) // if cmd_counter is still less than 10
        {
                idx = 0;
                id = 1;
        } else {
                idx = head;
                id = cmd_counter - HISTORY_LENGTH + 1;
        }
        printf("%d:\t%s", id, hist_cmd[idx]);
        idx = (idx + 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
        id++;
        while(idx != head) // iteration until array head
        {
                printf("%d:\t%s", id, hist_cmd[idx]);
                idx = (idx + 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
                id++;
        }
        fflush(0);
}
/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */
// inputBufferOri[] is added to record the original command before parsing
// fromSTDIN shows whether the inputBuffer[] comes from STDIN (user input) or from 

void setup(char inputBuffer[], char inputBufferOri[], char *args[],int *background,int fromSTDIN)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    ct = 0;
    /* read what the user enters on the command line */
// if the input is from stdin, then wait at read(); otherwise, directly parse
    if(fromSTDIN)
                length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
        else
                length = strlen(inputBuffer);
        // if the current input is <ctrl><c>, skip this command
        if(isSIGINT)
                return;
    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }
    strcpy(inputBufferOri, inputBuffer);
    /* examine every character in the inputBuffer */
    for (i = 0; i < length; i++) {
        switch (inputBuffer[i]){
        case ' ':
        case '\t' :               /* argument separators */
            if(start != -1){
                args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
        case '\n':                 /* should be the final char examined */
            if (start != -1){
                args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;
        case '&':
            *background = 1;
            inputBuffer[i] = '\0';
            break;
        default :             /* some other character */
            if (start == -1)
                start = i;
        }
    }
    args[ct] = NULL; /* just in case the input line was > 80 */
}
int main(void)
{
        /* set up the signal handler */
        struct sigaction handler;
        handler.sa_handler = handle_SIGINT;
        sigemptyset(&handler.sa_mask);
        handler.sa_flags = 0;
        sigaction(SIGINT, &handler, NULL);
        strcpy(buffer,"Caught <ctrl><c>\nRecent Commands\n");
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
        char inputBufferOri[MAX_LINE]; // record the original command input
    int background;             /* equals 1 if a command is followed by '&' */
    int status;
    char *args[MAX_LINE/2+1];/* command line (of 80) has max of 40 arguments */
    head = 0;
    cmd_counter = 0;
        memset(hist_cmd, 0, HISTORY_LENGTH*MAX_LINE);
    while (1){            /* Program terminates normally inside setup */
        isSIGINT = 0;
        background = 0;
        memset(inputBufferOri, 0, MAX_LINE); //reset the buffer array
        memset(inputBuffer, 0, MAX_LINE); //reset the buffer array
        printf("COMMAND->");
        fflush(0);
        setup(inputBuffer, inputBufferOri, args, &background, 1);       /* get next
command */
        if(isSIGINT)
                continue;
        // if input is "\n"
        if(args[0] == NULL)
                continue;
        // check the input format
        if (strcmp(args[0], "r") == 0)
        {
                if(args[1] == NULL) // the most recent command
                {
                        if(cmd_counter == 0)
                        {
                                printf("Error: no history command yet.\n");
                                continue;
                        }
                        // copy the last command into the head, and use setup() to 
                        int last = (head - 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
                        strcpy(hist_cmd[head], hist_cmd[last]);
                        head = (head + 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
                        strcpy(inputBuffer, hist_cmd[last]);
                        setup(inputBuffer, inputBufferOri, args, &background, 0);
                } else if(args[2] == NULL && strlen(args[1]) == 1){ // search the 
                        int idx = (head - 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
                        int hasFound = 0;
                        while(1)
                        {
                                if(hist_cmd[idx][0] == args[1][0])
                                {
                                        // copy the found command into the head, 
                                        strcpy(hist_cmd[head], hist_cmd[idx]);
                                        head = (head + 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
                                        strcpy(inputBuffer, hist_cmd[idx]);
                                        setup(inputBuffer, inputBufferOri, args, &background, 0);
                                        hasFound = 1;
                                        break;
                                }
                                if(idx == head)
                                        break;
                                idx = (idx - 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
                        }
                        if(!hasFound) // no mached command in record
                        {
                                printf("Error: no matched history command\n");
                                continue;
                        }
                } else {
                        printf("Error: Invalid Input. \"r ?\\n\" or \"r\\n\"\n");
                        continue;
                }
        } else {  // put the current command input history record
                strcpy(hist_cmd[head], inputBufferOri);
                head = (head + 1 + HISTORY_LENGTH) % HISTORY_LENGTH;
        }
        cmd_counter++;
// End of input parsing
// Start executing the command in a child process
        pid_t childpid = fork();
        if(childpid == 0) // child process
        {
                execvp(args[0], args);
                printf("ERROR COMMAND.\n");//show an error message.
                exit(0);
        }
        else if (childpid > 0) // parent process
        {
             if(background == 0) // if it is not a background cmd,
                                 // we have to wait for the child process
                waitpid(childpid,&status,0);
        }
        else  // fork() fails
        {
             perror("fork"); /* display error message */
        }
    }
}
