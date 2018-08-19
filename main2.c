#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>       
#include <unistd.h>
#include <sys/wait.h>

/**
THIS IS A MAIN TEST FOR SIGNALS, SIGUSR1 Y SIGUSR2 ARE TWO SIGNALS UNIX HAS FOR USERS TO OVERRIDE THEM
**/

/**
TO GENERATE SIGNALS, JUST TYPE IN ANOTHER TERMINAL WHILE THIS PROCESS IS RUNNING 'kill -signal pid'
 For example: kill -USR1 7517
**/

void parent_trap(int sig) {fprintf(stderr, "They got back together!\n");}
void child_trap(int sig) {fprintf(stderr, "Caught signal in CHILD.\n");}

int main(int argc, char **argv) {
    pid_t father = getpid();
    pid_t child;
    signal(SIGUSR2, parent_trap);
    pid_t process = fork();
    if (!process) {
        printf("HIJO: \n");
        child = getpid();
        printf("FORKEO SALIO PIOLA, pidHijo : %d\n",child);
        printf("FORKEO SALIO PIOLA, pidPadre : %d\n",father);

        signal(SIGUSR1, child_trap);
        int pauseRet = pause();
        printf("Pause retorno %d \n", pauseRet);
        raise(SIGUSR2);
    } else {
        int returnStatus;
        printf("PADRE: \n");

        waitpid(process, &returnStatus, 0);  // Parent process waits here for child to terminate.

        if (returnStatus == 0)  // Verify child process terminated without error.  
        {
        printf("The child process terminated normally.\n");    
        }

        if (returnStatus == 1)      
        {
        printf("The child process terminated with an error!.\n");    
        }
    }
}