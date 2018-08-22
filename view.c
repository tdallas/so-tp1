#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>


int main(int argc, char *argv[]){

	if(argc != 2){
		printf("Usage: ./view PID\n");
		return 1;
	}


	view(argv[1]);
	return 0;

}


void view(char *PID){


	//semaphore
	char semViewName[64];
	sem_t *semView, *semEnd;
	sprintf(semViewName, "semView%s", PID);
	semView = sem_open(semViewName, 0);

	if(semView == SEM_FAILED){
		perror("ERROR OPEN SEMAPHORE");
		return;
	}
	

	//shm for message size
	const int sharedMemorySize = sizeof(int); 
	const char* name = "MySharedMemory";
	int shm_fd;
	int* ptr;
	shm_fd = shm_open(name, O_RDONLY, 0666);
	ptr = (int*) mmap(NULL, sharedMemorySize, PROT_READ, MAP_SHARED, shm_fd, 0);


	sem_wait(semView);
	int msgSize = *ptr;
	
	//printf("%i\n",num);

	

	//
	//
	const int sharedMemorySize2 = msgSize; 
	const char* name2 = "MySharedMemory2";
	int shm_fd2;
	char* ptr2;
	shm_fd2 = shm_open(name2, O_RDONLY, 0666);

	ptr2 = (char*) mmap(NULL, sharedMemorySize2, PROT_READ, MAP_SHARED, shm_fd2, 0);

	
	fwrite(ptr2, sharedMemorySize2, 1, stdout);
	


	

	sem_close(semView);
	sem_unlink(semViewName);



}