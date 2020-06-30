/*
* Nicholas LaFramboise
* CSC 4421
* Take-home Lab #3
* This one was hard and took forever :(
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h> // Used for STDIN_FILENO and other POSIX
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 50
#define HIST_SIZE 10
#define MAX_LINE 80 // Ripped from lab 1

static char buffer[BUFFER_SIZE];
char cmdHist[HIST_SIZE][BUFFER_SIZE]; // Given the buffer size and knowing we need the previous 10 						 commands, we create a 2D char array to hold command history
int count = 0, caught = 0; // Counter for the 'global' history number and an int to flag for caught 				      signals
void loadHist()
{
  int histCount = 0; // Counter for the 'local' history number
  char histCommand[BUFFER_SIZE];
  FILE *histFile = fopen("./Nick's Command History - Lab 3", "r"); // Open file, second argument 									   specifies read only, and is 									   positioned at beginning of file
  if(histFile) // If file is open...
  {
    while(!feof(histFile)) // ...and while not at end-of-file...
    {
      strcpy(histCommand, "\0"); // ... copy the string null byte '\0' to the 					    'histCommand' buffer
      fgets(histCommand, MAX_LINE, histFile); // read from histFile and write to histCommand buffer, 							 stopping once EOF or newline is read
      if (strcmp(histCommand, "\0") == 1) // If the line in histCommand is 'empty' (has only a null 						     byte)...
        strcpy(cmdHist[histCount], histCommand); // copy histCommand->history element of index histCount
        histCount++; // Next histCount
	count++; // Increment global command count
    }
  }
}

void saveHist() // Saves the previous 10 commands
{
  FILE *histFile = fopen("./Nick's Command History - Lab 3", "w"); // Open command history file

  for(int i = 0; i < HIST_SIZE; i++) // Writes history to histFile
  {
    strcpy(buffer, cmdHist[i]); // Copy string from history[i] to buffer
    if(strcmp(buffer,"\0") != 0) // If the buffer is not null...
	fprintf(histFile,"%s", buffer); // ...print buffer to histFile
  }
}

void printHist() // This function prints the previous 10 commands
{
  int histCount = count, j = 0; // j is used as an index for history in the while loop
  printf("\nRecent Commands:\n");
  for (int i = 0; i < HIST_SIZE; i++) // Print the last 10 commands
  {
    printf("%d:   ",histCount); // Print the 'global' history number
    while ((cmdHist[i][j] != '\n') && (cmdHist[i][j] != '\0')) // While history is not a newline or 									  terminator character...
    {
      printf("%c",cmdHist[i][j]); // Print the current character in the history array 2D array
      j++; 			  // This while loop prints the characters of one command
    }
    printf("\n");
    j = 0; // Reset 'j' to the first character of the next command
    histCount--;
    if(histCount == 0) // Once ten commands have been printed, break the for loop
      break;
  }
  printf("\n");
  printf("COMMAND->"); // Query user for next command
  fflush(stdout);
}

void setup(char inputBuffer[], char *args[], int *background) // After halfway through developing this, 								 I decided to move the arguments and 									 code from main to a separate function 								         due to it being so large - then I 									 realized that most of this is from the 								 first takehome lab
{
  int cmdLength, start, ct = 0;

  cmdLength = read(STDIN_FILENO, inputBuffer, MAX_LINE); // Reads bytes from file descriptor 							    STDIN_FILENO into cmdLength, starting from 								    'inputBuffer'
  // This block was developed, not part of lab 1
  if (caught == 1) // If signal was caught...
  {
    cmdLength = read(STDIN_FILENO, inputBuffer, MAX_LINE);
    caught = 0; // Drop signal
  }

  if((strcmp(inputBuffer, "r\n\0") != 0) && (strncmp(inputBuffer, "r x", 2) != 0)) // Checks to see if r 										  or r x has been entered by user
  { 								       // if not, add to cmdHist
  for(int i = (HIST_SIZE - 1); i > 0; i--)
    strcpy(cmdHist[i], cmdHist[i-1]);
  
  strcpy(cmdHist[0], inputBuffer);
  count++;
  }

  // Most of this was from lab 1 minus the saveHist calls
  start = -1;
  if (cmdLength == 0)
  {
    saveHist();
    exit(0); /* ^d was entered, end of user command stream */
  }
  if ((cmdLength < 0) && (errno != EINTR))
  {
    perror("Error reading command.");
    saveHist();
    exit(-1); /* terminate with error code of -1 */
  }
	
  // If 'r' was entered, run the previous command
  if(strcmp(inputBuffer, "r\n") == 0)
    strcpy(inputBuffer, cmdHist[0]);
  // If 'r x' was entered, we have to find what command 'x' refers to
  else if(strncmp(inputBuffer, "r x", 2) == 0) // Notice that we used strncmp, not strcmp due to us 							  wanting the first char of x
  {
    for(int k = 0; k < 10; k++) // This for loop searches for char matches between x and the first char 				   of all past 10 cmdHist elements
    {
      if(inputBuffer[2] == cmdHist[k][0]) // As detailed in the lab, we use the first match we get
      {
        strcpy(inputBuffer, cmdHist[k]);
        break;
      }
    }
  }	
  cmdLength = strlen(inputBuffer);

  // Also ripped from lab 1
  for (int i = 0; i < cmdLength; i++)
  {
    switch (inputBuffer[i])
    {
      case ' ': case '\t': /* argument separators */
        if (start != -1)
        {
          args[ct] = &inputBuffer[start]; /* set up pointer */
          ct++;
        }
        inputBuffer[i] = '\0'; /* add a null char; make a C string */
        start = -1;
        break;

      case '\n': /* should be the final char examined */
        if (start != -1)
        {
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
				
      default: /* some other character */
	if (start == -1)
          start = i;
    }
  }
  args[ct] = NULL; /* just in case the input line was > 80 */
}

/* the signal handler function */
void handle_SIGINT()
{
	write(STDOUT_FILENO, buffer, strlen(buffer)); // Ripped from Lab 3, with a printHist addition
	printHist();
	caught = 1;
}

int main(void)
{
  // Declarations are ripped from lab 1
  char inputBuffer[MAX_LINE]; // Buffer to hold entered command
  int background, status, i; /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1];
  pid_t pid;
	
  for(i=0; i < HIST_SIZE; i++)
    memset(cmdHist[i],'\0', sizeof(cmdHist[i])); // memset fills memory with a constant byte - I made 							    that byte the null byte '\0'
  memset(inputBuffer, '\0', sizeof(inputBuffer)); // Set history and inputBuffers elements to null 

  // Given signal handler setup
  struct sigaction handler;
  handler.sa_handler = handle_SIGINT;
  sigemptyset(&handler.sa_mask);
  handler.sa_flags = 0;
  sigaction(SIGINT, &handler, NULL);
  strcpy(buffer, "Caught <ctrl><c>\n");

  loadHist(); // Load the command history

  while (1) // Nearly this whole while loop was ripped from lab 1
  {
    background = 0;
    strcpy(inputBuffer, "\0");
    printf("\nCOMMAND->");
    fflush(0);
    setup(inputBuffer, args, &background); // Get the next command
    fflush(0);
    pid = fork();
  
    if (pid < 0) // Failed to fork
    {
      fprintf(stderr, "Unable to properly fork."); // I couldnt get namespace std working for some reason
      saveHist();
      return 1;
    }
    else if(pid == 0) // Child
    {
      status = execvp(*args, args);
      if (status < 0)
      {
        fprintf(stderr, "Unable to execute %s", args[0]); // So I used fprintf instead of cerr
        saveHist();
        return 1;
      }
    }
    else if (background == 0)
      wait(NULL);
  }
  return 0; // FINALLY done
}
