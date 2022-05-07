
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

#include "include/colours.h"



void wsh_loop();
char *wsh_read_line();
char **wsh_split_line(char *line);
int wsh_launch(char **args);
int wsh_execute(char **args);

int wsh_cd(char **args);
int wsh_help(char **args);
int wsh_exit(char **args);

int wsh_num_builtins();

const char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

const int (*builtin_func[]) (char **) = {
  wsh_cd,
  wsh_help,
  wsh_exit
};



int main(int argc, char *argv[]){
  if(argc > 1){
    if((strcmp("-h", argv[1]) == 0) || (strcmp("--help", argv[1]) == 0)){
      wsh_help(argv);
      return 0;
    }
  }
  
  wsh_loop();
  return 0;
}



void wsh_loop(){
  char *line;
  char **args;
  int status;

  do {
    line = wsh_read_line();
    args = wsh_split_line(line);
    status = wsh_execute(args);

    free(args);
    free(line);
  } while(status);

}



char *wsh_read_line(){
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us
  char cwd[256];

  if(getcwd(cwd, sizeof(cwd)) == NULL){
    perror("getcwd() error");
    exit(1);
  }

  printf(B_CYAN "%s: $ " RESET, cwd);

  if(getline(&line, &bufsize, stdin) == -1){
    if(feof(stdin)){
      exit(0);  // We recieved an EOF
    } else  {
      perror("wsh");
      exit(1);
    }
  }

  return line;
}



#define WSH_TOK_BUFSIZE 64
#define WSH_TOK_DELIM " \t\r\n\a"
char **wsh_split_line(char *line){
  int bufSize = WSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufSize * sizeof(char*));
  char *token;

  if(!tokens){
    fprintf(stderr, "wsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, WSH_TOK_DELIM);
  while(token != NULL){
    tokens[position] = token;
    position++;

    if(position >= bufSize){
      bufSize += WSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufSize * sizeof(char*));
      if(!tokens){
        fprintf(stderr, "wsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, WSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}




int wsh_launch(char **args){
  pid_t pid, wpid;
  int status;

  pid = fork();
  if(pid == 0){
    // child process
    if(execvp(args[0], args) == -1){
      perror("wsh");
    }
    exit(1);
  } else if(pid < 0){
    // error forking
    perror("wsh");
  } else {
    // parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}



int wsh_execute(char **args){
  if (args[0] == NULL) {
    return 1; // empty command
  }

  // check if the command is implemented and run its corresponding function if it is
  for(int i = 0; i < wsh_num_builtins(); i++){
    if(strcmp(args[0], builtin_str[i]) == 0){
      return builtin_func[i](args);
    }
  }

  return wsh_launch(args);
}




int wsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}



int wsh_cd(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "wsh: expected argument to \"cd\"\n");
  } else {
    if(chdir(args[1]) != 0){
      perror("wsh");
    }
  }
  return 1;
}

int wsh_help(char **args){

  printf(GREEN "\n----------------------------------------\n\n");
  printf(B_WHITE "Usage: \n");
  printf("\n");
  printf(B_YELLOW "\t- Invoke shell:\n");
  printf(YELLOW   "\t  wsh\n");
  printf(B_YELLOW "\t- Current builtin commands:\n");
  for(int i = 0; i < wsh_num_builtins(); i++){
    printf(YELLOW "\t  %s\n", builtin_str[i]);
  }

  printf(GREEN "\n----------------------------------------\n\n" RESET);
  return 1;
}

int wsh_exit(char **args){
  return 0;
}
