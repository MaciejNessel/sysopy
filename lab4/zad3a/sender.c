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


void handle(int sig, siginfo_t *info, void *context){
    if(sig == SIGUSR1 || sig == SIGRTMIN + 1)
        counter++;
    if(sig == SIGUSR2 || sig == SIGRTMIN + 2){
        receiving = false;
    }
}

void send_SIGUSR1(char* mode, int my_sig1, pid_t catcher_pid, int n){
    for (int i = 0 ; i < n ; i++) {
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

    struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction = handle};
    sigaction(my_sig2, &act, NULL);
    if(sigaction(my_sig1, &act, NULL) == -1){
        printf("Error\n");
        exit(0);
    }
    if(sigaction(my_sig2, &act, NULL) == -1){
        printf("Error\n");
        exit(0);
    }

    send_SIGUSR1(mode, my_sig1, catcher_pid, n);
    send_SIGUSR2(mode, my_sig2, catcher_pid);

    while (receiving);

    printf("Sender otrzymał %d / %d signals\n", counter, n);

    return 0;
}