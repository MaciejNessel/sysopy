#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>



void siginfo_handler (int sig, siginfo_t *siginfo, void *context){
    printf ("Sending PID: %d, UID: %d\nReal user ID of sending process: %d\nSignal number being delivered: %d\nSignal code: %d\n",
            siginfo->si_pid, siginfo->si_uid, siginfo->si_uid, siginfo->si_signo, siginfo->si_code);
}

void signal_one(){
    printf("====SA_SIGINFO====\n");
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_handler = siginfo_handler;
    sigaction(SIGUSR1, &act, NULL);
    raise(SIGUSR1);
}



void nocldstop_handler(int signal){
    printf("notification from: SIG: %d, PID: %d, PPID: %d\n", signal, getpid(),getppid());
}

//SA_NOCLDSTOP
//        Jeśli   signum  jest  równe  SIGCHLD,  to  nie  są  odbierane  powiadomienia  o
//        zatrzymaniu procesu-dziecka (np. gdy dziecko otrzyma jeden z SIGSTOP,  SIGTSTP,
//        SIGTTIN  lub  SIGTTOU) ani o jego wznowieniu (np. po otrzymaniu SIGCONT) (patrz
//wait(2)). Flaga ta ma znaczenie tylko w przypadku  ustawiania  funkcji  obsługi
//sygnału SIGCHLD.
void signal_two(){
    printf("====SA_NOCLDSTOP====\n");
    struct sigaction act;
    act.sa_handler = &nocldstop_handler;
    sigaction(SIGCHLD, &act, NULL);

    pid_t child_pid = fork();
    if(child_pid == 0){
        while(1);
    }
    else{
        printf("SIGSTOP without flag\n");
        sleep(1);
        kill(child_pid, SIGSTOP);
        wait(NULL);
    }

    child_pid = fork();
    if(child_pid == 0){
        while(1);
    }
    else{
        printf("SIGKILL without flag\n");
        sleep(1);
        kill(child_pid, SIGKILL);
        wait(NULL);
    }

    act.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &act, NULL);

    child_pid = fork();
    if(child_pid == 0){
        while(1);
    }
    else{
        printf("SIGSTOP with flag\n");
        sleep(1);
        kill(child_pid, SIGSTOP);
        //wait(NULL);
    }
    child_pid = fork();
    if(child_pid == 0){
        while(1);
    }
    else{
        printf("SIGKILL with flag\n");
        sleep(1);
        kill(child_pid, SIGKILL);
        wait(NULL);
    }
}




void handler_SIGCHLD(int sig, siginfo_t *info, void *context) {
    printf("-----handler_SIGCHLD PID %d Child PID %d \n", getpid(), info->si_pid);
}
void send_signal_to_child(int sig){
    pid_t child_id = fork();

    if(child_id == 0) {
        while (1);
    } else {
        sleep(1);
        printf("%d sending %d to %d\n", getpid(), sig, child_id);

        kill(child_id, sig);

        fflush(stdout);
        sleep(1);
    }
}

void signal_three(){
    printf("\n************     RESETHAND TEST     *************\n");
    struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction = handler_SIGCHLD};
    sigaction(SIGCHLD, &act, NULL);

    printf("\nSA_NOCLDSTOP not set\n\n");

    send_signal_to_child(SIGKILL);
    send_signal_to_child(SIGKILL);


    fflush(stdout);
    sleep(1);

    printf("\nRESETHAND set\n\n");

    act.sa_flags = SA_RESETHAND | SA_SIGINFO;
    sigaction(SIGCHLD, &act, NULL);

    send_signal_to_child(SIGKILL);
    send_signal_to_child(SIGKILL);

    printf("flaga RESETHAND sprawila, ze po pierwszym sygnale handler zostal przywrocony do domyslnego\n");
}


int main(){

    //printf("Test first signal\n");
    //signal_one();

    //printf("Test second signal\n");
    //signal_two();

    //printf("Test third signal\n");
    signal_three();


}