#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <unistd.h>
#include "launcher.h"
#include <fcntl.h>

#define R 0
#define W 1

int openFile(const char *fileName, const char *mode)
{
   int flags;
   if (0 == strcmp("r", mode))
      flags = O_RDONLY;
   else if (0 == strcmp("w", mode))
      flags = O_WRONLY | O_CREAT | O_TRUNC;
   else {
      fprintf(stderr, "Unknown openFile mode %s\n", mode);
      exit(EXIT_FAILURE);
   }
   return open(fileName, flags, 0666);
}

void mydup2(int oldfd, int newfd)
{
   if (-1 == dup2(oldfd, newfd))
      MY_ERROR();
}

int dupMyFile(char *arg, int fileNO)
{
   int fd;
   if(arg == NULL)
      return 1;
   else if(fileNO == STDIN_FILENO)
      fd = openFile(arg, "r");
   else if(fileNO == STDOUT_FILENO)
      if(-1 == (fd = openFile(arg, "w"))){
         fprintf(stderr, "cshell: Unable to open file for output\n");
         exit(EXIT_FAILURE);
      }

   if (-1 == dup2(fd, fileNO))
      fprintf(stderr, "cshell: Unable to open file for %s\n",
         fileNO ? "output":"input");

   close(fd);

   return 0;
}

void childBranch(Shell *sh, int *fd_p, int *fd_c, int i)
{
   dupMyFile(sh->inFile[i], STDIN_FILENO);
   dupMyFile(sh->outFile[i], STDOUT_FILENO);

   if(i != 0)
      close(fd_c[R]);

   if (execvp(sh->args[i][0], sh->args[i]) == -1){
      fprintf(stderr, "cshell: %s: Command not found\n", sh->args[i][0]);
      exit(EXIT_FAILURE);
   }

}

void parentBranch(int *fd_p, int *fd_c, int i)
{
   if(i > 1)
      close((fd_p)[R]);
   (fd_p)[R] = (fd_c)[R];
   (fd_p)[W] = (fd_c)[W];
}

void forkSingleProcess(Shell* sh, int *fd_p, int *fd_c, int i){

   pid_t pid;

   close(fd_p[W]);

   if (pipe(fd_c) < 0){
      MY_ERROR();
   }

   if((pid = fork()) < 0){
      MY_ERROR();
   }

   else if(pid > 0)
      parentBranch(fd_p, fd_c, i);
   else {
      if(i != 0)
         mydup2(fd_p[R], STDIN_FILENO);

      mydup2(fd_c[W], STDOUT_FILENO);

      childBranch(sh, fd_p, fd_c, i);
   }
}

void lastProcessFork(Shell *sh, int *fd_p, int *fd_c, int i){

   pid_t pid;
   int status = 0;

   close(fd_c[W]);
   (fd_c)[W] = STDOUT_FILENO;

   if((pid = fork()) < 0){
      MY_ERROR();
   }

   else if(pid > 0 && i != 0)
      close((fd_p)[R]);

   else if(pid == 0) {
      if(i != 0)
         mydup2(fd_p[R], STDIN_FILENO);

      childBranch(sh, fd_p, fd_c, i);
   }
   while ((wait(&status)) > 0);
}

void runAllProcesses(Shell *sh, const int numProcess) {

   int i;
   int fd_p[2];
   int fd_c[2];

   fd_p[R] = fd_c[R] = STDIN_FILENO;
   fd_p[W] = fd_c[W] = 44;

   /* This contains the majority of the logic */
   for(i = 0; i < numProcess -1; i++)
      forkSingleProcess(sh, fd_p, fd_c, i);

   lastProcessFork(sh, fd_p, fd_c, i);
}
