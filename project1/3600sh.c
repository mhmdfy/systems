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
char* out = NULL;
char* in = NULL;

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

static void redirect(char* argv[]){
  int i = 0;
  while(argv[i] != NULL){
    if(strcmp(argv[i], ">") == 0){
      out = argv[i+1];
      argv[i] = NULL;
      break;
    }
    if(strcmp(argv[i], "<") == 0){
      in = argv[i+1];
      argv[i] = NULL;
      break;
    }
    i++;
  }
}

//reads the input from the user
static void getargs(char cmd[], int *argcp, char *argv[]){
  char *cmdp = cmd;
  char *end;
  int i = 0;
  in = NULL;
  out = NULL;

  // reading stdin and saving into cmd
  while((*cmdp = getc(stdin)) != '\n' || *cmdp == EOF){
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
  *argcp = i;
  redirect(argv);
}

static void execute(char *argv[]) {
  pid_t pid = fork();
  if(pid < 0) {
    printf("Error: Couldn't fork\n");
    exit(1);
  }
  else if(pid == 0) {
    if(out != NULL) {
      close(STDOUT_FILENO);
      if(open(out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR) < 0){
        printf("Error: Unable to open redirection file");
        exit(1);
      }
    }
    if(in != NULL) {
      close(STDIN_FILENO);
      if(open(in, O_RDONLY) < 0){
        printf("Error: Unable to open redirection file");
        exit(1);
      }
    }
    if(execvp(argv[0], argv) < 0){
      printf("Error: Command not found\n");
    }
    exit(1);
  }
  else {
    waitpid(pid, NULL, 0);
  } 
}

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 

  // Variables' declaration.
  char* cmd = (char*) calloc(MAX, sizeof(char));
  char* childargv[MAX/10];
  int childargc;
  char* user = (char*) calloc(50, sizeof(char));
  char* host = (char*) calloc(100, sizeof(char));
  char* dir = (char*) calloc(400, sizeof(char));

  // Main loop that reads a command and executes it
  while (1) {         
    // You should issue the prompt here
    user = getenv("USER");
    gethostname(host, 100);
    getcwd(dir, 400);
    printf("%s@%s:%s> ", user, host, dir);  
    fflush(stdout);
    // You should read in the command and execute it here
    getargs(cmd, &childargc, childargv);
    if (shouldExit || (childargc > 0 && strcmp(childargv[0], "exit") == 0)){
      do_exit();
    }
    execute(childargv);
  }
  free(cmd);
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
