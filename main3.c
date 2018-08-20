#include "queue2.h"
#include "md5.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>


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
    for (int i = 0 ; i < 4 ; i++) {
        num += (str[i] - '0') * exp;
        exp /= 10;
    }
    return num;
}


int main() {
    printf("%s\n", parseInt(4));
    printf("%d\n", parseChar("0005"));
}
