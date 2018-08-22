
#include "queue2.h"
#include "md5.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAX_SLAVES 4
#define BLOCK 6

typedef struct pipes {
  int pipeChildI[2];
  int pipeChildO[2];
} pipeType;

pipeType pipes[MAX_SLAVES];

char * parseInt(int num) {
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

  for (int i = 0 ; i < 4 ; i++)
  toRet[i] = (numArray[i] + '0');
  toRet[4] = '\0';
  return toRet;
}

int parseChar(char * str) {
  int num = 0;
  int exp = 1000;
  printf("supuesto bytes to read %s \n",str);
  for (int i = 0 ; i < 4 ; i++) {
    num += (str[i] - '0') * exp;
    exp /= 10;
  }
  return num;
}

//Delegates a task to the child specified by parameter
void delegateTask(int i){
  char *str1 = dequeue();

  write(pipes[i].pipeChildO[1], str1, strlen(str1)+1);
}

//Receive a task and pipes the md5 result to parent (Child)
void receiveTask(char *msgToRead, int i){

  char md5[MD5_LEN + 1];


  char * fullMsg;
  if(!CalcFileMD5(msgToRead, md5)){
    fullMsg = malloc(strlen("Error occurred!\n")+1);
    sprintf(fullMsg, "Error occurred!\n");
  }else{
    fullMsg = malloc(strlen(msgToRead)+strlen(" md5 \n")+1+strlen(md5));
    sprintf(fullMsg, "%s md5: %s\n", msgToRead, md5);
  }


  write(pipes[i].pipeChildI[1], fullMsg,strlen(fullMsg)+1);
  free(fullMsg);


}

//Reads from pipe if contains unread characters until finds a 0
char * readPipe(int pipe[2]){
  int index = 0;
  int size = BLOCK;
  char * msg = malloc(BLOCK);
  char buf;

  if(!(read(pipe[0], &buf, 1) > 0)){
    free(msg);
    return NULL;
  }

  msg[index++]=buf;
  while(buf != 0){
    if(read(pipe[0], &buf, 1) > 0){
      if(index +1 == size){
        msg = realloc(msg, size + BLOCK);
        size += BLOCK;
      }
      msg[index++] = buf;
    }
  }
  msg = realloc(msg, index+2);
  msg[index] = 0;
  return msg;
}


int main(int argc, char *argv[]){



  char *finalMsg = "";



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


  //
  //



  //lpthread lrt
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
  // memcpy(ptr2, finalMsg, finalMsgSize);


  printf("size queue %i\n", sizeQueue());
  printf("is empty %i\n", isEmpty());

  //Creates pipes to communicate with children (pipeChildI receives messages from child and pipeChildO sends messages to child)
  for (int i = 0 ; i < MAX_SLAVES ; i++) {
    pipe(pipes[i].pipeChildI);
    pipe(pipes[i].pipeChildO);
  }

  pid_t p[MAX_SLAVES];


  //int initial = sizeQueue()-1;
  for (int i = 0 ; i < MAX_SLAVES && sizeQueue()-1!=0 ; i++) {

    p[i] = fork();
    if (p[i] < 0) {
      fprintf(stderr, "Fork Failed" );
      return 1;
    } else if (p[i] == 0) {  // Child Process

      close(pipes[i].pipeChildI[0]);
      close(pipes[i].pipeChildO[1]);

      while(1){
        char * msg = readPipe(pipes[i].pipeChildO);
        if(msg != NULL){
          if(*msg == 0){
            printf("Child %d exit\n", i);
            close(pipes[i].pipeChildI[1]);
            close(pipes[i].pipeChildO[0]);
            exit(0);
          }else{
            receiveTask(msg, i);
          }
          free(msg);
        }
      }

    }else{ // Parent Process
      close(pipes[i].pipeChildI[1]);
      close(pipes[i].pipeChildO[0]);
    }
  }

  printf("so far so good, my pid is: %i\n", getpid());

  //Distribute inital tasks
  int sent =0, received = 0;
  int k=0, initialDistribution = (int)((sizeQueue()-1)/2);

  if(initialDistribution<MAX_SLAVES){
    for(int i=0; i< MAX_SLAVES && sizeQueue()-1>0; i++){
      delegateTask(i);
      sent++;
    }
  }else{
    while(initialDistribution > 0){
      delegateTask(k);
      k = (k+1)%(MAX_SLAVES);
      sent++;
      initialDistribution--;
    }
  }

  char str1[1];
  str1[0] = 0;
  while(sent != received){
    for(int i=0; i < MAX_SLAVES && sent != received ; i++){
      char * msg = readPipe(pipes[i].pipeChildI);
      if(msg != NULL){
        sprintf(ptr2, msg, strlen(msg)+1);
        ptr2+=strlen(msg)+1;
        received++;
        if(sizeQueue()-1!=0){
          sent++;
          delegateTask(i);
        }else{
          //Tell child that there are no more tasks
          write(pipes[i].pipeChildO[1], str1, 1);
        }
        free(msg);
      }
      printf("%d :: %d :: %d\n", received, sizeQueue()-1, sent);
    }
  }

  //Tells childs to exit and closes pipes

  for(int i=0; i<MAX_SLAVES;i++){
    write(pipes[i].pipeChildO[1], str1, 1);

    close(pipes[i].pipeChildI[0]);
    close(pipes[i].pipeChildO[1]);
  }




  sem_post(semView);





  sleep(20);
  printf("bye!\n");


  shm_unlink("MySharedMemory");
  shm_unlink("MySharedMemory2");
  sem_close(semView);
  sem_unlink(semViewName);



}
