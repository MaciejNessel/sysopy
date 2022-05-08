#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include "common.h"

struct pizza_place* table;

// Koñczy pracê dostawcy
void sigint_handler(int sig){
    if(shmdt(table) == -1){
        exit(1);
    }
    exit(0);
}

void block_access_to_table(struct sembuf *operations, int sem_id){
    // (wykonuje dekrementacjê semafora 3 - odpowiedzialnego za dostêp do sto³u)
    operations[0].sem_num = 3;
    operations[0].sem_op = -1;
    semop(sem_id, operations, 1);
}

void unlock_access_to_table(struct sembuf *operations, int sem_id){
    // (wykonuje inkrementacjê semafora 3 - odpowiedzialnego za dostêp do sto³u)
    operations[0].sem_num = 3;
    operations[0].sem_op = 1;
    semop(sem_id, operations, 1);
}

void take_pizza_from_table(struct sembuf *operations, int sem_id){
    table->types_of_pizza[table->take_from] = -1;
    table->take_from = (table->take_from+1) % TABLE_CAPACITY;
    operations[0].sem_num = 1;
    operations[0].sem_op = 1;
    semop(sem_id, operations, 1);
}


int main(int argc, char** argv){
    signal(SIGINT, sigint_handler);

    int table_id_memory = shmget(TABLE_KEY, 0, 0666);
    table = shmat(table_id_memory, NULL, 0);

    int sem_id = semget(SEM_KEY, 0, 0666);
    struct sembuf operations[1];

    int pizza_get, table_pizza_quantity;
    while(1){
        block_access_to_table(operations, sem_id);

        pizza_get = table->types_of_pizza[table->take_from];

        // Jeœli brak pizz na stole to odchodzimy od sto³u
        if(pizza_get == -1){
            unlock_access_to_table(operations, sem_id);
            continue;
        }

        take_pizza_from_table(operations, sem_id);

        table_pizza_quantity = TABLE_CAPACITY - semctl(sem_id, 1, GETVAL);
        print_time();
        printf(" Pobieram pizze: %d Liczba pizz na stole: %d\n", pizza_get, table_pizza_quantity);

        unlock_access_to_table(operations, sem_id);

        // Czas dojazdu do klienta [4-5s]
        sleep_random(4, 5);

        print_time();
        printf(" Dostarczam pizze: %d\n", pizza_get);

        // Czas powrotu [4-5s]
        sleep_random(4, 5);
    }

    return 0;
}