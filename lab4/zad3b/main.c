#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    pid_t catcher_pid;
    if ((catcher_pid = fork()) == 0){
        execl("./catcher", "./catcher", argv[2], NULL);
    }

    char * catcher_pid_str = malloc(10);
    sprintf(catcher_pid_str, "%d", catcher_pid);

    if (fork() == 0){
        execl("./sender", "./sender", argv[1], catcher_pid_str, argv[2], NULL);
    }

    for(int i=0; i<2; i++){
        wait(0);
    }

    return 0;

}