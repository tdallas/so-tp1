
#include "queue2.h"
#include "md5.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
 
int main(int argc, char *argv[]){

	if ( argc <2){
		printf("Usage: %s <directory>\n", argv[0]);
		return 0;
	} 

    //debugging
    printf("Creo la cola con parametros: %s \n", argv[1]);

	createPathQueue(argv[1]);

	printf("size queue %i\n", sizeQueue());
	printf("is empty %i\n", isEmpty());
	
    int fd1[2];  // Used to store two ends of first pipe
    int fd2[2];  // Used to store two ends of second pipe
    
    char input_str[100];
    pid_t p;

    char *str1 = dequeue();
   
 
    if (pipe(fd1)==-1)
    {
        fprintf(stderr, "Pipe Failed" );
        return 1;
    }
    if (pipe(fd2)==-1)
    {
        fprintf(stderr, "Pipe Failed" );
        return 1;
    }
 
    p = fork();
 
    if (p < 0)
    {
        fprintf(stderr, "fork Failed" );
        return 1;
    }
 
    // Parent process
    else if (p > 0)
    {
        char concat_str[100];
 
        close(fd1[0]);  // Close reading end of first pipe
 
        // Write input string and close writing end of first
        // pipe.
        write(fd1[1], str1, strlen(str1)+1);
        close(fd1[1]);
 
        // Wait for child to send a string
        wait(NULL);
 
        close(fd2[1]); // Close writing end of second pipe
 
        // Read string from child, print it and close
        // reading end.
        read(fd2[0], concat_str, 100);
        printf("string read father: %s\n", concat_str);
        close(fd2[0]);
    }
 
    // child process
    else
   		 {
    	    close(fd1[1]);  // Close writing end of first pipe
 
    	    // Read a string using first pipe

        	char concat_str[100];
        	read(fd1[0], concat_str, 100);
 
    	    // Close both reading ends
    	    close(fd1[0]);
    	    close(fd2[0]);
        
    	    printf("string read son %s\n", concat_str);
    	    // Write concatenated string and close writing end
      
    	    char md5[MD5_LEN + 1];
    	    
    	if (!CalcFileMD5(concat_str, md5)) {
    	    puts("Error occured!");
    	} else {
    	    printf("Success! MD5 sum is: %s\n", md5);
    	}

        write(fd2[1], md5, strlen(md5)+1);
    	    
        close(fd2[1]);
 
        exit(0);
    	}
}