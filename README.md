# SO - TP1
1)Download docker
> sudo apt-get install docker 
2) Download docker's image
> sudo docker pull agodio/itba-so:1.0
3) Go to the directory you want to link with docker's image and run docker:
> sudo docker run -v ${PWD}:/root -ti agodio/itba-so:1.0
4) Go to the project's root directory and compile:
> gcc -o tp main.c queue.c md5.c -lpthread -lrt
5) Have fun finding bugs:
> ./tp

*Step 4 will be override with a bash script*
