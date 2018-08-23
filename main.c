#include "queue.h"
#include "md5.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

struct msg {
    int bytesToRead;
    char * data;
} msg;

typedef struct pipes {
    int pipeChildI[2];
    int pipeChildO[2];
} pipeType;

char * parseInt(int num) {
    int i;
    int numArray[4];
    char *toRet = malloc(sizeof(char) *5);

    int current = 3;
    while (current != 0){
        if (num != 0) {
            numArray[current] = num % 10;
            num /=10;
        } else {
            numArray[current] = 0;
        }
        current-= 1;
    }

    for ( i = 0 ; i < 4 ; i++)
        toRet[i] = (numArray[i] + '0');
    toRet[4] = '\0';
    return toRet;
}

int parseChar(char * str) {
    int i;
    int num = 0;
    int exp = 1000;
    printf("supuesto bytes to read %s \n",str);
    for ( i = 0 ; i < 4 ; i++) {
        num += (str[i] - '0') * exp;
        exp /= 10;
    }
    return num;
}

int main(int argc, char *argv[]){


    int i;
   

    //semaphore
    char semViewName[64];
    sem_t *semView;
    sprintf(semViewName, "/semView%d", getpid());
    semView = sem_open(semViewName, O_CREAT | O_EXCL, 0777, 0);

    if(semView == SEM_FAILED){
        perror("ERROR OPEN SEMAPHORE");
        return 1;
    } 
    

	createPathQueue(argv[1]);

    int finalMsgSize = pathsSize() + ((sizeQueue()-1) * 41);
    

    //shm for size
    const int sharedMemorySize = sizeof(int*); 
    const char* name = "MySharedMemory";  
    int shm_fd; //shm file descriptor
    int *ptr;
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sharedMemorySize);
    ptr = (int*)mmap(NULL, sharedMemorySize, PROT_WRITE, MAP_SHARED, shm_fd, 0);

   
    memcpy(ptr, &finalMsgSize, sizeof(int));


    //shm for the message
    const int sharedMemorySize2 = finalMsgSize; 
    const char* name2 = "MySharedMemory2";
    int shm_fd2;
    char *ptr2;
    shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd2, sharedMemorySize2);
    
    ptr2 = (char*)mmap(NULL, sharedMemorySize2, PROT_WRITE, MAP_SHARED, shm_fd2, 0);


    

	//printf("size queue %i\n", sizeQueue());
	//printf("is empty %i\n", isEmpty());

    pipeType pipes[2];

    for ( i = 0 ; i < 2 ; i++) {
        pipe(pipes[i].pipeChildI);
        pipe(pipes[i].pipeChildO);
    }
    
    pid_t p[2];
    int originalSizeQueue = sizeQueue();

    FILE *file =fopen("md5files.txt", "a");

    for ( i = 0 ; i < 2 ; i++) {

        p[i] = fork();
        if (p[i] < 0) {
            fprintf(stderr, "fork Failed" );
            return 1;
        } else if (p[i] > 0) {  // Parent process
            close(pipes[i].pipeChildI[0]);  // Close reading end of first pipe

            // Write input string and close writing end of first
            // pipe.  
            while( sizeQueue()-1 != 0){
                if (i == 0 && sizeQueue() == originalSizeQueue/2)
                    break;
                char *str1 = dequeue();
                char *strlength = parseInt(strlen(str1));
                    
                struct msg *msgToSend = malloc(sizeof(msg));
                msgToSend->bytesToRead = strlen(str1);
                msgToSend->data = str1;

                write(pipes[i].pipeChildI[1], msgToSend, sizeof(msg));
                
                free(msgToSend->data);
                free(msgToSend);
            }

            close(pipes[i].pipeChildI[1]);
    
            close(pipes[i].pipeChildO[1]); // Close writing end of second pipe
            // Read string from child, print it and close
            // reading end.
            char buf;
            
            //printf("\nLectura desde padre:\n");
            while (read(pipes[i].pipeChildO[0], &buf, 1)> 0){
                //write(STDOUT_FILENO, &buf, 1); 
                //
                fprintf(file, "%c", buf);
                sprintf(ptr2, &buf);
                ptr2 += 1;         
            } 
                
            close(pipes[i].pipeChildO[0]);
        } else { // child process
                //printf("Proceso hijo\n");
                close(pipes[i].pipeChildI[1]);  // Close writing end of first pipe

                // Read a string using first pipe
                char md5[MD5_LEN + 1];
                struct msg * msgToRead = malloc(sizeof(msg));
                while (read(pipes[i].pipeChildI[0], msgToRead,sizeof(msg))> 0){

                    int len = msgToRead->bytesToRead;

                    if (!CalcFileMD5(msgToRead->data, md5)) {
                        puts("Error occured!");
                    } else {
                        // printf("Success! MD5 sum is: %s\n", md5);
                        write(pipes[i].pipeChildO[1], msgToRead->data, msgToRead->bytesToRead+1);
                        write(pipes[i].pipeChildO[1], " md5: ", 7);
                        write(pipes[i].pipeChildO[1], md5, strlen(md5));
                        write(pipes[i].pipeChildO[1], "\n", 1);
                        close(pipes[i].pipeChildO[0]);
                    }
                    free(msgToRead->data);
                    free(msgToRead);
                    msgToRead = malloc(sizeof(msg));
                }
                
                // Close reading end
                close(pipes[i].pipeChildI[0]);
                
                // Write concatenated string and close writing end
                close(pipes[i].pipeChildO[1]);
                exit(0);
            }	
    }
    int storage;
    for( i = 0; i < 2; i++)
    {
        if (p[i] != 0)
            waitpid(p[i], &storage, WUNTRACED);
    }


 
    fclose(file); 
    sem_post(semView);




    printf("so far so good, my pid is: %i\n", getpid());
    sleep(20);
    printf("bye!\n");


    shm_unlink("MySharedMemory");
    shm_unlink("MySharedMemory2");
    sem_close(semView);
    sem_unlink(semViewName);



 }
