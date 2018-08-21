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

	char semViewName[64];
	sem_t *semView, *semEnd;

	sprintf(semViewName, "semView%s", PID);
	

	semView = sem_open(semViewName, 0);
	

	if(semView == SEM_FAILED){
		perror("ERROR OPEN SEMAPHORE");
		return;
	}
	

	const int sharedMemorySize = 128; // --change size

	const char* name = "MySharedMemory";
	int shm_fd;
	void* ptr;

	shm_fd = shm_open(name, O_RDONLY, 0666);

	ptr = mmap(NULL, sharedMemorySize, PROT_READ, MAP_SHARED, shm_fd, 0);


	char * c = (char*)ptr;


	int i = sharedMemorySize;

	sem_wait(semView);
	while( i!= 0){
		printf("%c", *c);
			c++;
			i--;
	}


	sem_close(semView);
	sem_unlink(semViewName);



}