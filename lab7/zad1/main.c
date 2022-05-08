#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>
#include "common.h"

// Tablica przechowuj¹ca numery pid pracowaników (kucharzy oraz dostawców)
pid_t employees_pid[MAX_NO_WORKERS];
int no_employees = 0;

// Zamkniêcie pizzeri - do wszystkich potomków  procesu macierzystego (kucharzy/dostawców) jest wysy³any sygna³ SIGINT
void sigint_handler(int sig){
    for(int i=0; i<no_employees; i++){
        kill(employees_pid[i], SIGINT);
    }
}

int main(int argc, char** argv){
    if(argc != 3){
        puts("[main] Wrong number of arguments.");
        exit(1);
    }

    puts("Pizzeria jest otwierana...");
    sleep(1);

    int no_chefs = atoi(argv[1]);
    int no_suppliers = atoi(argv[2]);

    signal(SIGINT, sigint_handler);

    // Tworzy segment pamiêci wspólnej dla pieca
    int oven_id_memory = shmget(OVEN_KEY, sizeof(struct pizza_place), IPC_CREAT | 0666);
    if(oven_id_memory == -1){
        puts("[main] Oven shmget failed.");
        exit(1);
    }

    // Tworzy segment pamiêci wspólnej dla sto³u
    int table_id_memory = shmget(TABLE_KEY, sizeof(struct pizza_place), IPC_CREAT | 0666);
    if(table_id_memory == -1){
        puts("[main] Table shmget failed.");
        exit(1);
    }

    // Pobiera adres  pod którym do³¹czono segment pamiêci wspólnej.
    struct pizza_place* oven = shmat(oven_id_memory, NULL, 0);
    struct pizza_place* table = shmat(table_id_memory, NULL, 0);

    // Inicjalizacja pieca
    for(int i = 0; i < OVEN_CAPACITY; i++){
        oven->types_of_pizza[i] = -1;
    }
    oven->put_on = 0;
    oven->take_from = 0;

    // Inicjalizacja sto³u
    for(int i = 0; i < TABLE_CAPACITY; i++){
        table->types_of_pizza[i] = -1;
    }
    table->put_on = 0;
    table->take_from = 0;

    // Tworzymy zbiór semaforów. (liczba semaforów: 4)
    key_t sem_k = SEM_KEY;
    int sem_id_memory = semget(sem_k, 4, IPC_CREAT | 0666);

    // Semafory 0 i 1 gwarantuj¹, ¿e liczba pizz nie przekroczy pojemnoœci piekarnika / sto³u
    semctl(sem_id_memory, 0, SETVAL, OVEN_CAPACITY);
    semctl(sem_id_memory, 1, SETVAL, TABLE_CAPACITY);

    // Semafory 2 i 3 gwarantuj¹, ¿e piec / stó³ bêdzie obs³ugiwany max przez jedn¹ osobê
    semctl(sem_id_memory, 2, SETVAL, 1);
    semctl(sem_id_memory, 3, SETVAL, 1);

    pid_t child_pid;

    // Tworzy dostawców
    for(int i = 0; i < no_suppliers; i++){
        child_pid = fork();
        if(child_pid == 0){
            if(execlp("./supplier", "./supplier", NULL) == -1){
                puts("[main] Exec supplier failed.");
                exit(1);
            }
        }
        else{
            employees_pid[no_employees] = child_pid;
            no_employees++;
        }
    }

    // Tworzy kucharzy
    for(int i = 0; i < no_chefs; i++){
        child_pid = fork();
        if(child_pid == 0){
            if(execlp("./chef", "./chef", NULL) == -1){
                puts("[main] Exec chef failed.");
                exit(1);
            }
        }
        else{
            employees_pid[no_employees] = child_pid;
            no_employees++;
        }
    }

    // Oczekiwanie na zakoñczenie pracy pracowników (na sygna³ SIGING)
    for(int i = 0; i < no_chefs+no_suppliers; i++){
        wait(NULL);
    }

    // Od³¹cza semafory
    if(shmdt(oven) == -1){
        puts("[main] Closing oven shm failed");
        exit(1);
    }
    if(shmdt(table) == -1){
        puts("[main] Closing table shm failed");
        exit(1);
    }

    // Usuwa segmenty pamiêci wspólnej z systemu
    shmctl(table_id_memory, IPC_RMID, 0);
    shmctl(oven_id_memory, IPC_RMID, 0);
    semctl(sem_id_memory, IPC_RMID, 0);

    puts("\nPizzeria zamkniêta.");

    return 0;
}