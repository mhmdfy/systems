/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

#define MAX 300

int shouldExit = 0;

//get the first word for the input string
static char* getword(char * begin, char **endp){
  //gets rid of leading zeros
  while(*begin == ' '){
    begin++;
  }
  char* end = begin;
  //have end point to the end of the word
  while(*end != '\0' && *end != '\n' && *end != ' '){
    end++;
  }
  //null if we are at the end
  if(end == begin){
    return NULL;
  }
  //put a null char at the end of the word
  *end = '\0';
  *endp = end;
  return begin;
}

//reads the input from the user
static void getargs(char cmd[], int *argcp, char *argv[]){
  char *cmdp = cmd;
  char *end;
  int i = 0;

  // reading stin and saving into cmd, if couldnt read then exit
  while((*cmdp = getc(stdin) != '\n')){
    if(*cmdp == EOF){
      shouldExit = 1;
      break;
    }
    cmdp++;
  }

  cmdp = cmd;
  //scans through cmd and puts each word into argv
  while((cmdp = getword(cmdp, &end)) != NULL && cmdp[0] != '#'){
    argv[i] = cmdp;
    i++;
    cmdp = end+1;
  }
  argv[i] = cmdp;
  *argcp = i+1;
}

static void execute(int argc, char *argv[]) {
  int i = 0;
  for(i = 0; i < argc-1; i++) {
    printf("%d: %s ", i, argv[i]);
  }
  printf("\n");
}

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 

  // Variables' declaration.
  char* cmd = (char*) calloc(MAX, sizeof(char));
  char*childargv[MAX/10];
  int childargc;
  char* user = (char*) calloc(50, sizeof(char));
  char* host = (char*) calloc(100, sizeof(char));
  char* dir = (char*) calloc(400, sizeof(char));

  // Main loop that reads a command and executes it
  while (1) {         
    // You should issue the prompt here
    user = getlogin();
    gethostname(host, 100);
    getcwd(dir, 400);
    printf("%s@%s:%s> ", user, host, dir);  
    fflush(stdout);
    // You should read in the command and execute it here
    getargs(cmd, &childargc, childargv);
    if ((childargc > 0 && strcmp(childargv[0], "exit") == 0) || shouldExit){
      do_exit();
    }
    execute(childargc, childargv);
  }
  free(user);
  free(host);
  free(dir);
  return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
