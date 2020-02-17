#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <fcntl.h>
#include "launcher.h"

int checkChar(char ch, char *buf, unsigned *index)
{
   if((ch ==  '\n')) { /*check if char is newline*/
      ch = 0;
      buf[(*index)++] = ch;
      return 0;
   }
   else if(ch == EOF){
      ch = 0;
      buf[(*index)++] = ch;
      return 0;
   }
   else if((*index) == MAXLINE -2) {
      fprintf(stderr, "cshell: Command line too long\n");
      while(getchar() != '\n');
      return 1;
   }
   else
      buf[(*index)++] = ch;

   return 2;
}

int getInput(char * buf){

   unsigned index = 0;
   int state = 0;
   int ch;

   setbuf(stdout, NULL);
   while (EOF != (ch = getchar())){
      if(0 == (state = checkChar(ch, buf, &index)))
         break;
      else if (state == 1)
         return 0;
   }

   if(0 == index && feof(stdin)) {
      printf("exit\n");
      exit(EXIT_SUCCESS);
   }
   if(!strcmp(buf, "exit"))
      exit(EXIT_SUCCESS);

   return 1;
}

int checkToken(char **token, Shell *sh, int ComNum)
{
   if(*token == NULL)
      return 0;

   if(!strcmp(*token, "<")){
      sh->inFile[ComNum] = strtok(NULL, " ");
      *token = strtok(NULL, " ");
      return 1;
   }
   else if(!strcmp(*token, ">")){
      sh->outFile[ComNum] = strtok(NULL, " ");
      *token = strtok(NULL, " ");
      return 1;
   }
   return 0;
}
int addToArg(char **token, Shell *sh, int *command_num, int *arg_num)
{
   while(checkToken(token, sh, *command_num));
   if((*arg_num) == MAXARG){
      fprintf(stderr, "cshell: %s: Too many arguments\n",
         sh->args[*command_num][0]);
      return 1;
   }
   if((*token) != NULL)
      sh->args[*command_num][(*arg_num)++] = (*token);
   (*token) = strtok(NULL, " ");
   return 0;
}

int parsePipeArgs(Shell *sh, int *command_num)
{
   char *token;
   int arg_num = 0;

   token = strtok(sh->args[*command_num][0], " ");
   while( token != NULL )
      if(addToArg(&token, sh, command_num, &arg_num))
         return 1;

   if(arg_num == 0){
      fprintf(stderr, "cshell: Invalid pipe\n");
      return 1;
   }

   (*command_num)++;
   arg_num = 0;

   return 0;
}
int countPipes(char * buf)
{
   int i =0;
   int pipe_count = 0;

   while(buf[i] != '\0'){
      if(buf[i] == '|')
         pipe_count++;
      i++;
   }

   return pipe_count;
}

/*
 * This function parses the inputs taken from get_input()
 *
 * and transforms it into an 2-dimentional matrix, of (char *)'s
 *
 * This gives us an array of pointers with many entries
 * simmilar to char ** argv.
 * This is all made to seporate inputs delimiated by pipes,
 * and spaces at the same time.
 */
int parseInputs(char * buf, Shell *sh){

   int command_num = 0;
   int pipe_num = 0;
   char *token;

   pipe_num = countPipes(buf);

   /* separate pipes first */
   token = strtok(buf, "|");
   while( token != NULL ) {
      sh->args[command_num++][0] = token;
      token = strtok(NULL, "|");
   }

   if(command_num > MAXCOMMAND){
      fprintf(stderr, "cshell: Too many commands\n");
      return 0;
   }

   /* Then parse commands themselves */
   command_num = 0;
   while(sh->args[command_num][0] != NULL)
      if(parsePipeArgs(sh, &command_num))
         return 0;

   if((command_num != pipe_num + 1) && (pipe_num != 0)){
      fprintf(stderr, "cshell: Invalid pipe\n");
      return 0;
   }

   return command_num;
}

void shell_loop() {

   char buf[MAXLINE];
   Shell sh;
   int command_count;

   while(1){
      memset(sh.args, 0, MAXCOMMAND * MAXARG * sizeof(char *));
      memset(sh.inFile, 0, MAXCOMMAND * sizeof(char *));
      memset(sh.outFile, 0, MAXCOMMAND * sizeof(char *));
      memset(buf, 0, 1025);

      printf(":-) ");
      if(getInput(buf)){
         command_count = parseInputs(buf, &sh);
         if(command_count)
            runAllProcesses(&sh, command_count);
      }
   }
}

int main(int argc, char *argv[]) {

   setbuf(stdout, NULL);
   shell_loop();
   exit(EXIT_SUCCESS);
}

