#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

bool receiving = true;
int counter = 0;
pid_t sender_pid;


void handle_SIGUSR1(int sig){
    printf("--- catcher got %d\n", sig);
    counter+=1;
}

void handle_SIGUSR2(int sig, siginfo_t *info, void *context){
    receiving = false;
    sender_pid = info->si_pid;
}

void send_SIGUSR1(char* mode, int my_sig1){
    // Wysyłanie odebranych sygnałów do sender
    for (int i = 0 ; i < counter ; i++){
        if (strcmp(mode, "SIGQUEUE") == 0) {
            union sigval val;
            sigqueue(sender_pid, my_sig1, val);
        } else {
            kill(sender_pid, my_sig1);
        }
    }
}

void sendSIGUSR2(char* mode, int my_sig2){
    if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval val;
        sigqueue(sender_pid, my_sig2, val);
    } else {
        kill(sender_pid, my_sig2);
    }
}

int main(int argc, char* argv[]){
    printf("Catcher PID: %d\n", getpid());

    char* mode = argv[1];

    int my_sig1 = SIGUSR1;
    int my_sig2 = SIGUSR2;

    if (strcmp(mode, "SIGRT") == 0){
        my_sig1 = SIGRTMIN + 1;
        my_sig2 = SIGRTMIN + 2;
    }

    signal(my_sig1, handle_SIGUSR1);
    struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction = handle_SIGUSR2};
    sigaction(my_sig2, &act, NULL);

    // Oczekiwanie na sygnały - przechwyca dopóki nie otrzyma SIGUSR2
    while (receiving);

    send_SIGUSR1(mode, my_sig1);
    sendSIGUSR2(mode, my_sig2);

    printf("catcher received %d signals\n", counter);

    return 0;
}