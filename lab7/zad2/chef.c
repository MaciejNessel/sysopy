#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

struct pizza_place* oven;
struct pizza_place* table;
sem_t* table_quantity;
sem_t* table_access;
sem_t* oven_quantity;
sem_t* oven_access;

void sigint_handler(int sig){
    munmap(oven, OVEN_CAPACITY*sizeof(int));
    munmap(table, TABLE_CAPACITY*sizeof(int));
    sem_close(table_quantity);
    sem_close(oven_quantity);
    sem_close(table_access);
    sem_close(oven_access);
    exit(0);
}


int main(int argc, char** argv){
    signal(SIGINT, sigint_handler);

    // Otwiera segment pamięci wspólnej (dla pieca) oraz określa rozmiar
    int oven_id_memory = shm_open(OVEN, O_RDWR, 0666);
    if(oven_id_memory < 0){
        puts("Creating oven shm failed");
        exit(1);
    }
    ftruncate(oven_id_memory, OVEN_CAPACITY*sizeof(int));

    // Otwiera segment pamięci wspólnej (dla stołu) oraz określa rozmiar
    int table_id_memory = shm_open(TABLE, O_RDWR, 0666);
    if(table_id_memory < 0){
        puts("Creating table shm failed");
        exit(1);
    }
    ftruncate(table_id_memory, TABLE_CAPACITY*sizeof(int));

    // Dołącza otwarte segmenty pamięci wspólnej do przestrzeni adresowej procesu
    oven = mmap(NULL, sizeof(struct pizza_place), PROT_READ | PROT_WRITE, MAP_SHARED, oven_id_memory, 0);
    table = mmap(NULL, sizeof(struct pizza_place), PROT_WRITE | PROT_READ, MAP_SHARED, table_id_memory, 0);

    // Otwiera semafory odpowiedzialne za kontrolę liczby pizz / dostępu do stołu, pieca
    table_quantity = sem_open(S_TABLE_QUANTITY, O_RDWR, 0666);
    oven_quantity = sem_open(S_OVEN_QUANTITY, O_RDWR, 0666);
    table_access = sem_open(S_TABLE_ACCESS, O_RDWR, 0666);
    oven_access = sem_open(S_OVEN_ACCESS, O_RDWR, 0666);

    srand(getpid());

    int pizza_type, space_on_table, space_in_oven, pizza_from_oven;
    while(1){
         // Losuje typ pizzy [0-9]
        pizza_type = rand() % 10;

        print_time();
        printf(" Przygotowuję pizzę: %d\n", pizza_type);

        // Losuje czas przygotowania pizzy [1-2s]
        sleep_random(1, 2);

        sem_wait(oven_access);
        sem_wait(oven_quantity);

        oven->types_of_pizza[((oven->put_on++) % OVEN_CAPACITY)] = pizza_type;
        sem_getvalue(oven_quantity, &space_in_oven);

        print_time();
        printf(" Dodałem pizze: %d Liczba pizz w piecu: %d\n",pizza_type, OVEN_CAPACITY - space_in_oven);

        sem_post(oven_access);

        // Czas pieczenia [4-5s]
        sleep_random(4, 5);

        sem_wait(oven_access);

        pizza_from_oven = oven->types_of_pizza[oven->take_from];
        oven->types_of_pizza[oven->take_from] = -1;
        oven->take_from = (oven->take_from+1) % OVEN_CAPACITY;
        sem_post(oven_quantity);

        sem_post(oven_access);

        sem_wait(table_quantity);
        table->types_of_pizza[(table->put_on++) % TABLE_CAPACITY] = pizza_from_oven;
        sem_getvalue(oven_quantity, &space_in_oven);
        sem_getvalue(table_quantity, &space_on_table);

        print_time();
        printf(" Wyjmuje pizze: %d Liczba pizz w piecu: %d Liczba pizz na stole: %d\n",
               pizza_type, OVEN_CAPACITY - space_in_oven, TABLE_CAPACITY - space_on_table);
        }

    return 0;
}