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

struct pizza_place* table;
sem_t* table_quantity;
sem_t* table_access;

void sigint_handler(int sig){
    munmap(table, TABLE_CAPACITY*sizeof(int));
    sem_close(table_quantity);
    sem_close(table_access);
    exit(0);
}

int main(int argc, char** argv){
    signal(SIGINT, sigint_handler);

    // Otwiera segment pamiêci wspólnej (dla sto³u) oraz okreœla rozmiar
    int table_id_memory = shm_open(TABLE, O_RDWR, 0666);
    if(table_id_memory < 0){
        puts("Problem with creating shm for table");
        exit(1);
    }
    ftruncate(table_id_memory, TABLE_CAPACITY*sizeof(int));

    // Do³¹cza otwarty segment pamiêci wspólnej do przestrzeni adresowej procesu
    table = mmap(NULL, sizeof(struct pizza_place), PROT_WRITE | PROT_READ, MAP_SHARED, table_id_memory, 0);

    // Otwiera semafory odpowiedzialne za kontrolê liczby pizz / dostêpu do sto³u
    table_quantity = sem_open(S_TABLE_QUANTITY, O_RDWR, 0666);
    table_access = sem_open(S_TABLE_ACCESS, O_RDWR, 0666);

    int pizza_get, space_on_table;
    while(1){
        sem_wait(table_access);

        pizza_get = table->types_of_pizza[table->take_from];

        // Jeœli brak pizz na stole to odchodzimy od sto³u
        if(pizza_get == -1){
            sem_post(table_access);
            continue;
        }

        table->types_of_pizza[table->take_from] = -1;
        table->take_from = (table->take_from+1) % TABLE_CAPACITY;
        sem_post(table_quantity);
        sem_getvalue(table_quantity, &space_on_table);
        print_time();
        printf(" Pobieram pizze: %d Liczba pizz na stole: %d\n",pizza_get, TABLE_CAPACITY - space_on_table);

        sem_post(table_access);

        // Losuje czas dostawy [4-5s]
        sleep_random(4, 5);

        print_time();
        printf(" Dostarczam pizze: %d\n", pizza_get);

        // Losuje czas powrotu [4-5s]
        sleep_random(4, 5);
    }

    return 0;
}