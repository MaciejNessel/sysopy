#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Wrong number of arguments.\n");
        return 0;
    }
    int n = atoi(argv[1]);

    pid_t pid;
    printf("PID procesu macierzystego: %d\n", getpid());

    for (int i = 0; i < n; i++){
        pid = fork();
        if (pid == 0) {
            printf("My PID = %d  | PID Parent = %d\n", getpid(), getppid());
            kill(getpid(), SIGTERM);
        } else if(pid == -1){
            printf("Error in fork.\n");
            exit(1);
        }
    }

    return 0;

}