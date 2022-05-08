#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

pid_t employees_pid[MAX_NO_WORKERS];
int no_employees;

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
    no_employees = 0;

    signal(SIGINT, sigint_handler);

    // Tworzymy segment pami�ci wsp�lnej dla pieca, a nast�pnie okre�lenie rozmiaru
    int oven_id_memory = shm_open(OVEN, O_CREAT | O_RDWR, 0666);
    if(oven_id_memory < 0){
        perror("Problem with creating shm for oven\n");
        exit(1);
    }
    ftruncate(oven_id_memory, OVEN_CAPACITY*sizeof(int));

    // Tworzymy segment pami�ci wsp�lnej dla sto�u, a nast�pnie okre�lenie rozmiaru
    int table_id_memory = shm_open(TABLE, O_CREAT | O_RDWR, 0666);
    if(table_id_memory < 0){
        perror("Problem with creating shm for table\n");
        exit(1);
    }
    ftruncate(table_id_memory, TABLE_CAPACITY*sizeof(int));

    // Pobieramy adresy do��czonych segment�w
    struct pizza_place* oven_state = mmap(NULL, sizeof(struct pizza_place), PROT_WRITE, MAP_SHARED, oven_id_memory, 0);
    struct pizza_place* table_state = mmap(NULL, sizeof(struct pizza_place),  PROT_WRITE, MAP_SHARED, table_id_memory, 0);

    // Inicjalizacja pieca
    for(int i = 0;i < OVEN_CAPACITY; i++){
        oven_state->types_of_pizza[i] = -1;
    }
    oven_state->put_on = 0;
    oven_state->take_from = 0;

    // Inicjalizacja sto�u
    for(int i = 0; i < TABLE_CAPACITY; i++){
        table_state->types_of_pizza[i] = -1;
    }
    table_state->put_on = 0;
    table_state->take_from = 0;

    // Semafory, kt�re gwarantuj�, �e liczba pizz nie przekroczy pojemno�ci piekarnika / sto�u
    sem_t* table_space = sem_open(S_TABLE_QUANTITY, O_CREAT | O_RDWR, 0666, TABLE_CAPACITY);
    sem_t* oven_space = sem_open(S_OVEN_QUANTITY, O_CREAT | O_RDWR, 0666, OVEN_CAPACITY);

    // Semafory, kt�re gwarantuj�, �e piec / st� b�dzie obs�ugiwany max przez jedn� osob�
    sem_t* table_lock = sem_open(S_TABLE_ACCESS, O_CREAT | O_RDWR, 0666, 1);
    sem_t* oven_lock = sem_open(S_OVEN_ACCESS, O_CREAT | O_RDWR, 0666, 1);

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


    // Zamykamy semafory
    sem_close(table_space);
    sem_close(oven_space);
    sem_close(table_lock);
    sem_close(oven_lock);

    // Usuwamy semafory
    sem_unlink(S_TABLE_QUANTITY);
    sem_unlink(S_OVEN_QUANTITY);
    sem_unlink(S_TABLE_ACCESS);
    sem_unlink(S_OVEN_ACCESS);

    // Od��cza segmenty pami�ci wsp�lnej od przestrzeni adresowej procesu
    munmap(oven_state, OVEN_CAPACITY*sizeof(int));
    munmap(table_state, TABLE_CAPACITY*sizeof(int));

    // Oznacza segment jako do usuni�cia
    shm_unlink(TABLE);
    shm_unlink(OVEN);

    return 0;
}