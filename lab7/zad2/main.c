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
    no_employees = 0;

    signal(SIGINT, sigint_handler);

    // Tworzymy segment pamiêci wspólnej dla pieca, a nastêpnie okreœlenie rozmiaru
    int oven_id_memory = shm_open(OVEN, O_CREAT | O_RDWR, 0666);
    if(oven_id_memory < 0){
        perror("Problem with creating shm for oven\n");
        exit(1);
    }
    ftruncate(oven_id_memory, OVEN_CAPACITY*sizeof(int));

    // Tworzymy segment pamiêci wspólnej dla sto³u, a nastêpnie okreœlenie rozmiaru
    int table_id_memory = shm_open(TABLE, O_CREAT | O_RDWR, 0666);
    if(table_id_memory < 0){
        perror("Problem with creating shm for table\n");
        exit(1);
    }
    ftruncate(table_id_memory, TABLE_CAPACITY*sizeof(int));

    // Pobieramy adresy do³¹czonych segmentów
    struct pizza_place* oven_state = mmap(NULL, sizeof(struct pizza_place), PROT_WRITE, MAP_SHARED, oven_id_memory, 0);
    struct pizza_place* table_state = mmap(NULL, sizeof(struct pizza_place),  PROT_WRITE, MAP_SHARED, table_id_memory, 0);

    // Inicjalizacja pieca
    for(int i = 0;i < OVEN_CAPACITY; i++){
        oven_state->types_of_pizza[i] = -1;
    }
    oven_state->put_on = 0;
    oven_state->take_from = 0;

    // Inicjalizacja sto³u
    for(int i = 0; i < TABLE_CAPACITY; i++){
        table_state->types_of_pizza[i] = -1;
    }
    table_state->put_on = 0;
    table_state->take_from = 0;

    // Semafory, które gwarantuj¹, ¿e liczba pizz nie przekroczy pojemnoœci piekarnika / sto³u
    sem_t* table_space = sem_open(S_TABLE_QUANTITY, O_CREAT | O_RDWR, 0666, TABLE_CAPACITY);
    sem_t* oven_space = sem_open(S_OVEN_QUANTITY, O_CREAT | O_RDWR, 0666, OVEN_CAPACITY);

    // Semafory, które gwarantuj¹, ¿e piec / stó³ bêdzie obs³ugiwany max przez jedn¹ osobê
    sem_t* table_lock = sem_open(S_TABLE_ACCESS, O_CREAT | O_RDWR, 0666, 1);
    sem_t* oven_lock = sem_open(S_OVEN_ACCESS, O_CREAT | O_RDWR, 0666, 1);

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

    // Od³¹cza segmenty pamiêci wspólnej od przestrzeni adresowej procesu
    munmap(oven_state, OVEN_CAPACITY*sizeof(int));
    munmap(table_state, TABLE_CAPACITY*sizeof(int));

    // Oznacza segment jako do usuniêcia
    shm_unlink(TABLE);
    shm_unlink(OVEN);

    return 0;
}