
#include "queue2.h"
#include "md5.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

#define MAX_SLAVES 5
#define BLOCK 10

//NO LO ESTOY USANDO
struct msg {
  int bytesToRead;
  char * data;
};

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

  if (!CalcFileMD5(msgToRead, md5)) {
    puts("Error occured!");
  } else {

    char * fullMsg = malloc(strlen(msgToRead)+6+strlen(md5));
    sprintf(fullMsg, "%s md5:%s", msgToRead, md5);

    write(pipes[i].pipeChildI[1], fullMsg,strlen(fullMsg)+1);

    free(fullMsg);
  }

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

  if ( argc <2){
    printf("Usage: %s <directory>\n", argv[0]);
    return 0;
  }

  createPathQueue(argv[1]);

  printf("size queue %i\n", sizeQueue());
  printf("is empty %i\n", isEmpty());

  //Creates pipes to communicate with children (pipeChildI receives messages from child and pipeChildO sends messages to child)
  for (int i = 0 ; i < MAX_SLAVES ; i++) {
    pipe(pipes[i].pipeChildI);
    pipe(pipes[i].pipeChildO);
  }

  pid_t p[MAX_SLAVES];

  for (int i = 0 ; i < MAX_SLAVES && sizeQueue()-1!=0 ; i++) {

    p[i] = fork();
    if (p[i] < 0) {
      fprintf(stderr, "fork Failed" );
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

      delegateTask(i);
    }
  }

  while(sizeQueue()-1 != 0){
    for(int i=0; i < MAX_SLAVES && sizeQueue()-1 != 0 ; i++){
      char * msg = readPipe(pipes[i].pipeChildI);
      if(msg != NULL){
        printf("(%d) %s\n",i, msg);
        delegateTask(i);
        free(msg);
      }
    }
  }

  //Tell childs to exit and closes pipes
  char str1[1];
  str1[0] = 0;
  for(int i=0; i<MAX_SLAVES;i++){
    write(pipes[i].pipeChildO[1], str1, 1);

    close(pipes[i].pipeChildI[0]);
    close(pipes[i].pipeChildO[1]);
  }



  //NO SE QUE HACE
  int storage;
  for(int i = 0; i < 2; i++)
  {
    if (p[i] != 0)
    waitpid(p[i], &storage, WUNTRACED);
  }
}
