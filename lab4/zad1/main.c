#include "stdio.h"
#include "stdlib.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>



// Za pomocą sigpending odczytujemy listę sygnałów, które oczekują na odblokowanie w danym procesie
// Następnie sprawdzamy czy w zestawie znajduje się sygnał SIGUSR1
void pending_signal() {
    sigset_t pending_set;
    sigpending(&pending_set);

    if (sigismember(&pending_set, SIGUSR1)) {
        printf("SIGUSR1 jest obsługiwany.\n");
    } else{
        printf("SIGUSR1 NIE jest obsługiwany.\n");
    }
}


// handler obsługujący sygnał wypisujący komunikat o jego otrzymaniu
void handle_signal() {
    printf("Sygnał został przechwycony i obsłużony.\n");
}


// Tworzymy potomka funkcją fork ponownie przy pomocy funkcji raise potomek wysyła sygnał do samego siebie
// (z wyjątkiem opcji pending, gdzie testowane jest sprawdzenie, czy sygnał czekający w przodku jest widoczny w potomku).
void fork_test(char* operation_mode) {
    if (fork() == 0) {
        printf("Start fork test: \n");
        if (strcmp(operation_mode, "pending") != 0) {
            raise(SIGUSR1);
        } else {
            pending_signal();
        }
        printf("End fork test.\n");
        exit(0);
    } else{
        wait(0);
    }
}


char** create_new_arg(char **argv){
    char** new_argv = calloc(4, sizeof (char*));
    for (int i = 0; i < 2; i++) {
        new_argv[i] = calloc(sizeof(char), strlen(argv[i]));
        strcpy(new_argv[i], argv[i]);
    }
    new_argv[2] = "CHILD_EXEC";
    new_argv[3] = NULL;
    return new_argv;
}

void exec_test_start(char **argv){
    if(fork() == 0){
        char** new_argv = create_new_arg(argv);
        execv(argv[0], new_argv);
    }
    else{
        wait(0);
    }
}


void exec_test(char* operation_mode){
    printf("\nStart EXEC test:\n");
    if (strcmp(operation_mode, "pending") != 0)
        raise(SIGUSR1);
    else
        pending_signal();
    printf("End EXEC test.\n");
}


int main(int argc, char** argv) {
    // Walidacja argumentów wejściowych
    char* operation_mode = calloc(10, sizeof (char));
    if(strcmp(argv[1], "ignore") != 0 && strcmp(argv[1], "handler") != 0 && strcmp(argv[1], "mask") != 0 && strcmp(argv[1], "pending") != 0){
        printf("Wprowadzono niepoprawny argument!\n");
        return 0;
    } else{
        operation_mode = argv[1];
    }

    if (argc == 3 && strcmp(argv[2], "CHILD_EXEC") == 0) {
        exec_test(operation_mode);
        return 0;
    }

    printf("Start test [mode: %s]\n", operation_mode);
    if(strcmp(operation_mode, "ignore") == 0){
        signal(SIGUSR1, SIG_IGN);
    }
    else if(strcmp(operation_mode, "handler") == 0){
        signal(SIGUSR1, handle_signal);
    }
    else if(strcmp(operation_mode, "mask") == 0 || strcmp(operation_mode, "pending") == 0){
        sigset_t blockingSet;
        sigemptyset(&blockingSet);
        sigaddset(&blockingSet, SIGUSR1);
        sigprocmask(SIG_BLOCK, &blockingSet, NULL);
    }
    raise(SIGUSR1);

    if (strcmp(operation_mode, "pending") == 0){
        pending_signal();
    }
    printf("\n");
    fork_test(operation_mode);
    exec_test_start(argv);

    return 0;
}

