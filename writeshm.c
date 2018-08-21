#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char **argv){


const int sharedMemorySize = 128; //--change size 
const char* name = "MySharedMemory";
//shm file descriptor
int shm_fd;
void *ptr;
shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
ftruncate(shm_fd, sharedMemorySize);
ptr = mmap(NULL, sharedMemorySize, PROT_WRITE, MAP_SHARED, shm_fd, 0);




char semViewName[64];
sem_t *semView;

sprintf(semViewName, "/semView%d", getpid());


semView = sem_open(semViewName, O_CREAT | O_EXCL, 0777, 0);


if(semView == SEM_FAILED){
	perror("ERROR OPEN SEMAPHORE");
	return 1;
} 

char *msg = "MENSAJE\nMENSAJE\nMENSAJE\n";
sprintf(ptr, msg);
ptr += strlen(msg);
sem_post(semView);




printf("so far so good, my pid is: %i\n", getpid());
sleep(20);
printf("bye!\n");


shm_unlink("MySharedMemory");
sem_close(semView);
sem_unlink(semViewName);


}

//-lpthread -lrt 
