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

// Tablica przechowuj�ca numery pid pracowanik�w (kucharzy oraz dostawc�w)
pid_t employees_pid[MAX_NO_WORKERS];
int no_employees = 0;

// Zamkni�cie pizzeri - do wszystkich potomk�w  procesu macierzystego (kucharzy/dostawc�w) jest wysy�any sygna� SIGINT
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

    // Tworzy segment pami�ci wsp�lnej dla pieca
    int oven_id_memory = shmget(OVEN_KEY, sizeof(struct pizza_place), IPC_CREAT | 0666);
    if(oven_id_memory == -1){
        puts("[main] Oven shmget failed.");
        exit(1);
    }

    // Tworzy segment pami�ci wsp�lnej dla sto�u
    int table_id_memory = shmget(TABLE_KEY, sizeof(struct pizza_place), IPC_CREAT | 0666);
    if(table_id_memory == -1){
        puts("[main] Table shmget failed.");
        exit(1);
    }

    // Pobiera adres  pod kt�rym do��czono segment pami�ci wsp�lnej.
    struct pizza_place* oven = shmat(oven_id_memory, NULL, 0);
    struct pizza_place* table = shmat(table_id_memory, NULL, 0);

    // Inicjalizacja pieca
    for(int i = 0; i < OVEN_CAPACITY; i++){
        oven->types_of_pizza[i] = -1;
    }
    oven->put_on = 0;
    oven->take_from = 0;

    // Inicjalizacja sto�u
    for(int i = 0; i < TABLE_CAPACITY; i++){
        table->types_of_pizza[i] = -1;
    }
    table->put_on = 0;
    table->take_from = 0;

    // Tworzymy zbi�r semafor�w. (liczba semafor�w: 4)
    key_t sem_k = SEM_KEY;
    int sem_id_memory = semget(sem_k, 4, IPC_CREAT | 0666);

    // Semafory 0 i 1 gwarantuj�, �e liczba pizz nie przekroczy pojemno�ci piekarnika / sto�u
    semctl(sem_id_memory, 0, SETVAL, OVEN_CAPACITY);
    semctl(sem_id_memory, 1, SETVAL, TABLE_CAPACITY);

    // Semafory 2 i 3 gwarantuj�, �e piec / st� b�dzie obs�ugiwany max przez jedn� osob�
    semctl(sem_id_memory, 2, SETVAL, 1);
    semctl(sem_id_memory, 3, SETVAL, 1);

    pid_t child_pid;

    // Tworzy dostawc�w
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

    // Oczekiwanie na zako�czenie pracy pracownik�w (na sygna� SIGING)
    for(int i = 0; i < no_chefs+no_suppliers; i++){
        wait(NULL);
    }

    // Od��cza semafory
    if(shmdt(oven) == -1){
        puts("[main] Closing oven shm failed");
        exit(1);
    }
    if(shmdt(table) == -1){
        puts("[main] Closing table shm failed");
        exit(1);
    }

    // Usuwa segmenty pami�ci wsp�lnej z systemu
    shmctl(table_id_memory, IPC_RMID, 0);
    shmctl(oven_id_memory, IPC_RMID, 0);
    semctl(sem_id_memory, IPC_RMID, 0);

    puts("\nPizzeria zamkni�ta.");

    return 0;
}