#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


// Test 1. Flaga: SA_SIGINFO
//    Funkcja  obsługi  sygnałów pobiera trzy argumenty, a nie jeden. W typ przypadku
//    zamiast ustawiać sa_handler należy ustawić sa_sigaction. Flaga ta ma  znaczenie
//    tylko w przypadku ustanawiania funkcji obsługi sygnału.
void handler_SIGINFO (int sig, siginfo_t *siginfo, void *context){
    printf(">> handler_SIGINFO [PID %d] [Child PID %d] \n\n", getpid(), siginfo->si_pid);
}

void test_SIGINFO(){
    printf("#__________TEST: SA_SIGINFO__________#\n");
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    raise(SIGUSR1);
}


void send_signal_to_child(int sig){
    pid_t child_pid = fork();

    if(child_pid == 0) {
        while (1);
    } else {
        sleep(1);
        printf("[Parent PID: %d] sending %d to [Child PID: %d]\n", getpid(), sig, child_pid);
        kill(child_pid, sig);
        sleep(1);
    }
}


// Test 2. Flaga: SA_NOCLDSTOP
//    Jeśli   signum  jest  równe  SIGCHLD,  to  nie  są  odbierane  powiadomienia  o
//    zatrzymaniu procesu-dziecka (np. gdy dziecko otrzyma jeden z SIGSTOP,  SIGTSTP,
//    SIGTTIN  lub  SIGTTOU) ani o jego wznowieniu (np. po otrzymaniu SIGCONT) (patrz
//    wait(2)). Flaga ta ma znaczenie tylko w przypadku  ustawiania  funkcji  obsługi
//    sygnału SIGCHLD.
void handler_NOCLDSTOP(int sig, siginfo_t *siginfo, void *context){
    printf(">> handler_NOCLDSTOP [PID %d] [Child PID %d] \n\n", getpid(), siginfo->si_pid);
}

void test_NOCLDSTOP(){
    printf("#__________TEST: SA_NOCLDSTOP__________#\n");

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = &handler_NOCLDSTOP;

    if (sigaction(SIGCHLD, &act, NULL)){
        printf("Signal could not be handled\n");
        exit(1);
    }

    printf("_____ SA_NOCLDSTOP flag is not set _____\n");
    send_signal_to_child(SIGSTOP);

    act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &act, NULL);

    printf("_______ SA_NOCLDSTOP flag is set _______\n");
    send_signal_to_child(SIGSTOP);

}


// Test 3. Flaga: SA_RESETHAND
//    Odtwarza akcję  sygnałową  do  stanu  domyślnego  po  wejściu  funkcji  obsługi
//    sygnału.  Flaga  ta ma znaczenie tylko w przypadku ustanawiania funkcji obsługi
//    sygnału. SA_ONESHOT jest przestarzałym, niestandardowym synonimem tej flagi.
void handler_SIGCHLD(int sig, siginfo_t *siginfo, void *context) {
    printf(">> handler_SIGCHLD PID %d Child PID %d \n", getpid(), siginfo->si_pid);
}

void test_RESETHAND(){
    printf("\n#__________TEST: SA_RESETHAND__________#\n");

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler_SIGCHLD;
    sigaction(SIGCHLD, &act, NULL);

    printf("_____ SA_RESETHAND flag is not set _____\n");
    send_signal_to_child(SIGKILL);
    send_signal_to_child(SIGKILL);


    printf("_______ SA_RESETHAND flag is set _______\n");
    act.sa_flags = SA_RESETHAND;
    sigaction(SIGCHLD, &act, NULL);

    send_signal_to_child(SIGKILL);
    send_signal_to_child(SIGKILL);

    printf("RESETHAND sprawiła, ze po pierwszym sygnale handler zostal przywrócony do domyślnego\n");
}


int main(){
    test_SIGINFO();
    test_NOCLDSTOP();
    test_RESETHAND();
    return 0;
}