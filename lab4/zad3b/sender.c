#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>


bool receiving = true;
int counter = 0;
bool waiting = false;

void handle_SIGUSR1(int sig){
    printf("sender got %d\n", sig);
    waiting = false;
    counter+=1;
}

void handle_SIGUSR2(int sig, siginfo_t *info, void *context){
    receiving = false;
}

void send_SIGUSR1(char* mode, int my_sig1, pid_t catcher_pid, int n){
    for (int i = 0 ; i < n ; i ++) {
        while (waiting);

        waiting = true;

        if (strcmp(mode, "SIGQUEUE") == 0) {
            union sigval val;
            sigqueue(catcher_pid, my_sig1, val);
        } else {
            kill(catcher_pid, my_sig1);
        }
    }
}

void send_SIGUSR2(char* mode, int my_sig2, pid_t catcher_pid){
    if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval val;
        sigqueue(catcher_pid, my_sig2, val);
    } else {
        kill(catcher_pid, my_sig2);
    }
}


int main(int argc, char* argv[]){
    int n = atoi(argv[1]);
    pid_t catcher_pid = (pid_t) atoi(argv[2]);
    char* mode = argv[3];

    int my_sig1 = SIGUSR1;
    int my_sig2 = SIGUSR2;

    if (strcmp(mode, "SIGRT") == 0){
        my_sig1 = SIGRTMIN + 1;
        my_sig2 = SIGRTMIN + 2;
    }

    signal(my_sig1, handle_SIGUSR1);
    struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction = handle_SIGUSR2};
    sigaction(my_sig2, &act, NULL);

    send_SIGUSR1(mode, my_sig1, catcher_pid, n);
    send_SIGUSR2(mode, my_sig2, catcher_pid);


    // Oczekiwanie na SIGUSR2
    while (receiving);

    printf("sender received %d/%d signals\n", counter, n);

    return 0;
}