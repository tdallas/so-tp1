
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

	createPathQueue(argv[1]);

	printf("size queue %i\n", sizeQueue());
	printf("is empty %i\n", isEmpty());
	
    int fd1[2];  // Used to store two ends of first pipe
    int fd2[2];  // Used to store two ends of second pipe
 
    char input_str[100];
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
        char concat_str[100];
 
        close(fd1[0]);  // Close reading end of first pipe
 
        // Write input string and close writing end of first
        // pipe.  
        while( sizeQueue()-1 != 0){
            char *str1 = dequeue();
            write(fd1[1], str1, strlen(str1)+1);
            write(fd1[1], "///", 3);    
        }

        close(fd1[1]);

        // Wait for child to send a string
        wait(NULL);
 
        close(fd2[1]); // Close writing end of second pipe
        // Read string from child, print it and close
        // reading end.
        char buf;
        printf("\nlectura desde padre:\n");
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

            char buf;
            char str[500];
        	while (read(fd1[0], &buf, 1)> 0){
                strcat(str, &buf);

                printf("string: %s\n", str);

                int len = strlen(str);
                const char *last_tree = &str[len]-3;
                if( strcmp(last_tree, "///") == 0  ){
                
                    str_cut(str, strlen(str)-3, 3 );
                    printf("SLASH, string: %s  \n", str ); 

                    char md5[MD5_LEN + 1];

                    if (!CalcFileMD5(str, md5)) {
                        puts("Error occured!");
                    } else {
                        printf("Success! MD5 sum is: %s\n", md5);
                        write(fd2[1], str, strlen(str)+1);
                        write(fd2[1], " md5: ", 6);
                        write(fd2[1], md5, strlen(md5)+1);
                        write(fd2[1], "|", 1);
                        close(fd2[0]);
                    }

                    strcpy(str, "");
                }
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
