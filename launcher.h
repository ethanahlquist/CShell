#ifndef LAUNCHER_H
#define LAUNCHER_H

#define MAXLINE 1025
#define MAXCOMMAND 20
#define MAXARG 11

typedef struct {
   char * args[MAXCOMMAND][MAXARG];
   char * inFile[MAXCOMMAND];
   char * outFile[MAXCOMMAND];
} Shell;

void runAllProcesses(Shell *, const int);

void launchCommands(Shell *);

#define MIN(A,B) (((A) < (B)) ? (A):(B))
#define MAX(A,B) (((A) > (B)) ? (A):(B))

#define MY_ERROR() { perror(NULL); exit(EXIT_FAILURE); }

/*
#define MY_ERROR() \
{ \
   perror(NULL); \
   exit(EXIT_FAILURE); \
}
*/

#define MY_MALLOC(_ptr,_size) \
    if (NULL == ((_ptr) = malloc(_size))) { \
           fprintf(stderr, "Failed to allocate memory, in %s at line %d.\n"\
            ,__FILE__, __LINE__); \
           exit(EXIT_FAILURE);\
        }


#define MY_CALLOC(_ptr,_num,_type) \
   do { \
       if (NULL == ((_ptr) = calloc(_num, sizeof(_type)))) { \
              fprintf(stderr, "Failed to allocate memory, in %s at line %d.\n"\
               ,__FILE__, __LINE__); \
              exit(EXIT_FAILURE);\
           } \
   } while(0)

#endif

