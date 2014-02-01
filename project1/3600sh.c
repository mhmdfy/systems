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

// Global Variables.
int SHOULD_EXIT = 0;
char* OUT = NULL;
char* IN = NULL;
char* ERR = NULL;
int IS_BACKGROUND = 0;
int INVALID = 0;
int BAD_ESCAPE = 0;

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 

  // Variables' declaration.
  char* cmd;
  char* childargv[MAX/10];
  int childargc;
  char* user = (char*) calloc(50, sizeof(char));
  char* host = (char*) calloc(100, sizeof(char));
  char* dir = (char*) calloc(400, sizeof(char));

  // Main loop that reads a command and executes it.
  while (1) {         
    if(SHOULD_EXIT){
      do_exit();
    }
    // Get user information.
    user = getenv("USER");
    gethostname(host, 100);
    getcwd(dir, 400);

    // Get the command from the Prompts.
    cmd = (char*) calloc(MAX, sizeof(char));
    printf("%s@%s:%s> ", user, host, dir);  
    fflush(stdout);
    getargs(cmd, &childargc, childargv);

    // If the command is "exit", then exit.
    if ((childargc > 0 && strcmp(childargv[0], "exit") == 0)){ 
      do_exit();
    }
    // check to see if the syntax is valid before executing
    if(INVALID){
      printf("Error: Invalid syntax.\n"); 
    }
    else if(BAD_ESCAPE){
      printf("Error: Unrecognized escape sequence.\n");
    }
    else{
      // Execute the command.
      execute(childargv);
    }
    free(cmd);
  }
  // UNREACHABLE 
  return 1;
}

// Reads the input from the user.
void getargs(char cmd[], int *argcp, char *argv[]){
  char *cmdp = cmd;
  char *end;
  int i = 0;
  char *wordp;

  // Set file variables to null.
  IN = NULL;
  OUT = NULL;
  ERR = NULL;
  IS_BACKGROUND = 0;
  INVALID = 0;
  BAD_ESCAPE = 0;

  // Reading stdin and saving into cmd.
  while((*cmdp = getc(stdin)) != '\n'){
    if(*cmdp == EOF){
      SHOULD_EXIT = 1;
      break;
    }
    cmdp++;
  }

  // If we are at the end of the file (there is no command) exit.
  cmdp = cmd;
  if(*cmdp == EOF) do_exit();

  // Scans through cmd and puts each word into argv.
  while((wordp = getword(cmdp, &end)) != NULL && wordp[0] != '#'){
    argv[i] = wordp;
    i++;
    cmdp = end+1;
  }

  argv[i] = wordp;
  *argcp = i;
  
  // Figure out the file redirection characters.
  background(argv);
  redirect(argv);
}

// Get the first word for the input string
char* getword(char * begin, char **endp){
  int i = 0;
  char* word = (char*) calloc(MAX, sizeof(char));

  // Gets rid of leading zeros
  while(*begin == ' ' || *begin == '\t'){
    begin++;
  }

  if(*begin == '&'){
   word[0] = '&';
   word[1] = '\0';
   *endp = begin;
   return word;
  }

  // Have end point to the end of the word
  char* end = begin;
  while(*end != '\0' && *end != '\n' && *end != ' ' && *end != EOF && *end != '&' && *end != '\t'){
    if(*end == '\\'){//escape characters
      end++;
      word[i] = escape(*end);
    }
      
    else{
      word[i] = *end;
    }
    end++;
    i++;
  }

  // Return null if we are at the end; There are no words
  if(end == begin){
    return NULL;
  }

  if(*end == '&'){
    end--;
  }

  // Put a null char at the end of the word
  word[i+1] = '\0';
  *endp = end;
  return word;
}

// Search the arguments for file redirection, and there are any,
// save them to the correct global variable.
void redirect(char* argv[]){
  int i = 0;
  int outC = 0;
  int inC = 0;
  int errC = 0;

  while(argv[i] != NULL){
    if(strcmp(argv[i], ">") == 0){
      if(outC == 0 && !redirectChar(argv[i+1]) && redirectChar(argv[i+2])){
        OUT = argv[i+1];
        argv[i] = NULL;
        outC++;
      }
      else{
        INVALID = 1;
      }
    }
    else if(strcmp(argv[i], "2>") == 0){
      if(errC == 0 && !redirectChar(argv[i+1]) && redirectChar(argv[i+2])){
        ERR = argv[i+1];
        argv[i] = NULL;
        errC++;
      }
      else {
        INVALID = 1;
      }
    }  
    else if(strcmp(argv[i], "<") == 0){
      if(inC == 0 && !redirectChar(argv[i+1]) && redirectChar(argv[i+2])){
        IN = argv[i+1];
        argv[i] = NULL;
        inC++;
       }
       else {
         INVALID = 1;
       }
    }
    i++;
  }
}


int redirectChar(char* file) {
  if(file == NULL)
    return 1;
  if(strcmp(file, ">") == 0 || strcmp(file, "<") == 0 
   || strcmp(file, "2>") == 0 || strcmp(file, "&") == 0) 
    return 1;
  else
    return 0;
}

// searches the arguments for background processes
// if there is change the background indicator to true,
// if & is incorrectly used print invalid
void background(char* argv[]){
  int i = 0;
  while(argv[i] != NULL){
    if(strcmp(argv[i], "&") == 0){
      if(argv[i+1] == NULL){
        IS_BACKGROUND = 1;
        argv[i] = NULL;
      }
      else{
        INVALID = 1;
      }
    }
    i++;
  }
}

// handles escape characters
char escape(char c){
  if(c == 't')
    return '\t';
  if(c == '\\' || c == ' ' || c == '&')
    return c;
  else {
    BAD_ESCAPE = 1;
    return c;
  }  
}

// Executes the command by forking a child and waiting for it.
void execute(char *argv[]) {
  pid_t pid = fork();
  if(pid < 0) {
    printf("Error: Couldn't fork\n");
    exit(1);
  }
  else if(pid == 0) {
    if(OUT != NULL) {
      close(STDOUT_FILENO);
      if(open(OUT, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR) < 0){
        printf("Error: Unable to open redirection file.\n");
        exit(1);
      }
    }
    if(IN != NULL) {
      close(STDIN_FILENO);
      if(open(IN, O_RDONLY) < 0){
        printf("Error: Unable to open redirection file.\n");
        exit(1);
      }
    }
    if(ERR != NULL) {
      close(STDERR_FILENO);
      if(open(ERR, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR) < 0){
        printf("Error: Unable to open redirection file.\n");
        exit(1);
      }
    }
    if(execvp(argv[0], argv) < 0){
      if(errno == EACCES)
        printf("Error: %s.\n", strerror(errno));
      else
        printf("Error: Command not found.\n");
    }
    exit(1);
  }
  else {
    if(!IS_BACKGROUND){
      waitpid(pid, NULL, 0);//not background
    }
  } 
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
