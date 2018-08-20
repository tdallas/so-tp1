
#include "queue2.h"
#include "md5.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

struct msg {
    int bytesToRead;
    char * data;
} msg;

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

int main(int argc, char *argv[]){

	if ( argc <2){
		printf("Usage: %s <directory>\n", argv[0]);
		return 0;
	} 

	createPathQueue(argv[1]);

	printf("size queue %i\n", sizeQueue());
	printf("is empty %i\n", isEmpty());
	
    int fd1[2];  // Used to store two ends of first pipe
    int fd2[2];  // Used to store two ends of second pipe
 
    pid_t p;

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
        close(fd1[0]);  // Close reading end of first pipe
 
        // Write input string and close writing end of first
        // pipe.  
        while( sizeQueue()-1 != 0){
            char *str1 = dequeue();
            // printf("String: %s, strlen: %d \n", str1, strlen(str1));
            char *strlength = parseInt(strlen(str1));
                
            struct msg *msgToSend = malloc(sizeof(msg));
            msgToSend->bytesToRead = strlen(str1);
            msgToSend->data = str1;
            // printf("A punto de hacer un write \n");
            // printf("Voy a escribir un string de %d bytes, string : %s\n", msgToSend->bytesToRead, msgToSend->data);

            // write(fd1[1], msgToSend.bytesToRead, sizeof(int));
            // write(fd1[1], msgToSend->data, msgToRead->bytes);
            write(fd1[1], msgToSend, sizeof(msg));
            
            free(msgToSend->data);
            free(msgToSend);
        }

        close(fd1[1]);

        // Wait for child to send a string
        wait(NULL);
 
        close(fd2[1]); // Close writing end of second pipe
        // Read string from child, print it and close
        // reading end.
        char buf;
        printf("\nLectura desde padre:\n");
        while (read(fd2[0], &buf, 1)> 0){
            write(STDOUT_FILENO, &buf, 1); 
        }   
        close(fd2[0]);
    }
    // child process
    else
   		 {
    	    close(fd1[1]);  // Close writing end of first pipe
 
    	    // Read a string using first pipe

            char md5[MD5_LEN + 1];
            struct msg * msgToRead = malloc(sizeof(msg));
        	while (read(fd1[0], msgToRead,sizeof(msg))> 0){

                int len = msgToRead->bytesToRead;

                if (!CalcFileMD5(msgToRead->data, md5)) {
                    puts("Error occured!");
                } else {
                    printf("Success! MD5 sum is: %s\n", md5);
                    write(fd2[1], msgToRead->data, msgToRead->bytesToRead+1);
                    write(fd2[1], " md5: ", 7);
                    write(fd2[1], md5, strlen(md5));
                    write(fd2[1], "\n", 1);
                    close(fd2[0]);
                }
                free(msgToRead->data);
                free(msgToRead);
                msgToRead = malloc(sizeof(msg));
            }
            
    	    // Close reading end
    	    close(fd1[0]);
    	    
            // Write concatenated string and close writing end
      	    close(fd2[1]);
    	}	
}

/*
 *      Remove given section from string. Negative len means remove
 *      everything up to the end.
 */
int str_cut(char *str, int begin, int len)
{
    int l = strlen(str);

    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}
