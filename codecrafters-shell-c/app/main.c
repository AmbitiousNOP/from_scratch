#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

int read_dir(char abs_path[], char cmd[]){
  DIR *d;
  struct dirent *dir;
  d = opendir(abs_path);
  if(d){
    while ((dir = readdir(d)) != NULL){
      if (strcmp(dir->d_name, cmd)==0){
        printf("%s is %s/%s\n", cmd, abs_path, cmd);
        closedir(d);
        return 0;
      }
    }
    closedir(d);
  }
  return 1;
}

int check_builtin(char *req_builtin_check, char *list_of_builtins[], int num_of_builtins){

  for (int i = 0; i < num_of_builtins; i++){
    if (strcmp(list_of_builtins[i], req_builtin_check) == 0){
      printf("%s is a shell builtin\n", list_of_builtins[i]);
      return 0;
    }
  }
  return 1;
}

int main() {
  char *path = getenv("PATH");
  //TODO: error handling 
  const char match[2] = ":";

  char *builtins[] = {"exit", "type", "echo"};
  int num_of_builtins = sizeof(builtins) / sizeof(builtins[0]);
  
  while(1){

    // Uncomment this block to pass the first stage
    printf("$ ");
    fflush(stdout);

    // Wait for user input
    char input[100] = {0};
    fgets(input, 100, stdin);

    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n'){
      input[len-1] = '\0';
    }

    // copy chars up until white space (grabs the command)
    char command[100] = {0};
    int i;
    for (i = 0; i < len; i++){
      if (input[i] == ' '){
        break;
      }else{
        command[i] = input[i];
      }
    }
    command[i] = '\0';

    if (strcmp(command, "exit") == 0){
      return 0;
    }else if (strcmp(command, "echo") == 0){
      printf("%s\n", input+5);
    }else if (strcmp(command, "type") == 0){
      if (check_builtin(input+5, builtins, num_of_builtins)==0){
        continue;
      }
      //printf("DEBUG type match.. looking for %s\n", input+5);
      
      char path_copy[1000];
      strncpy(path_copy, path, sizeof(path_copy));
      path_copy[sizeof(path_copy) - 1] = '\0';

      char *token = strtok(path_copy, match);
      int found = 0;
      while (token != NULL){
        if(read_dir(token, input+5)==0){
          found = 1;
          break;
        }
        token = strtok(NULL, ":");
      }
      if(!found){
        printf("%s: not found\n", input+5);
      }
    }else{
      printf("%s: command not found\n", input);
    }

  }

  return 0;
}
