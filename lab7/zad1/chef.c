#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include "common.h"


struct pizza_place* oven;
struct pizza_place* table;

// Kończy pracę kucharza
void sigint_handler(int sig){
    if(shmdt(oven) == -1 || shmdt(table) == -1){
        exit(1);
    }
    exit(0);
}

void block_access_to_oven(struct sembuf *operations, int sem_id){
    // Blokuje dostęp do pieca dla pozostałych kucharzy - wkładamy do pieca
    // (wykonuje dekrementacje semafora 2 - odpowiedzialnego za dostęp do pieca)
    operations[0].sem_num = 2;
    operations[0].sem_op = -1;
    semop(sem_id, operations, 1);
}

void unlock_access_to_oven(struct sembuf *operations, int sem_id){
    // Odblokowuje dotęp do pieca dla pozostałych kucharzy
    // (wykonuje inkrementację semafora 2 - odpowiedzialnego za dostęp do pieca)
    operations[0].sem_num = 2;
    operations[0].sem_op = 1;
    semop(sem_id, operations, 1);
}


void put_pizza_in_oven(struct sembuf *operations, int pizza_type, int sem_id){
    // Wkłada pizzę do pieca
    // (wykonuje dekrementacje semafora 0 - odpowiedzialnego za kontrolę liczby pizz w piecu)
    operations[0].sem_num = 0;
    operations[0].sem_op = -1;
    semop(sem_id, operations, 1);
    oven->types_of_pizza[(oven->put_on++) % OVEN_CAPACITY] = pizza_type;
}

int take_out_pizza_of_oven(struct sembuf *operations, int sem_id){
    int pizza_from_oven = oven->types_of_pizza[oven->take_from];
    oven->types_of_pizza[oven->take_from] = -1;
    oven->take_from = (oven->take_from+1) % OVEN_CAPACITY;
    // Po wyciągnięciu pizzy zwiększa liczbę wolnego miejsca w piecu
    // (wykonuje inkrementację semafora 0 - odpowiedzialnego za kontrolę liczby pizz w piecu)
    operations[0].sem_num = 0;
    operations[0].sem_op = 1;
    semop(sem_id, operations, 1);

    return pizza_from_oven;
}

void put_pizza_on_table(struct sembuf *operations, int pizza_from_oven, int sem_id){
    // Wyciągnięto pizzę z pieca, aktualizujemy liczbę pizz na stole
    operations[0].sem_num = 1;
    operations[0].sem_op = -1;
    semop(sem_id, operations, 1);
    table->types_of_pizza[(table->put_on++) % TABLE_CAPACITY] = pizza_from_oven;
}

int main(int argc, char** argv){
    signal(SIGINT, sigint_handler);

    // Uzyskuje identyfikator pamięci wspólnej (stołu i pieca)
    int table_id_memory = shmget(TABLE_KEY, 0, 0666);
    int oven_id_memory = shmget(OVEN_KEY, 0, 0666);

    // Pobiera adres powyższych pamięci wspólnych
    oven = shmat(oven_id_memory, NULL, 0);
    table = shmat(table_id_memory, NULL, 0);

    // Pobiera zbiór semaforów utworzonych w ./main
    int sem_id = semget(SEM_KEY, 0, 0666);

    // Tworzy tablice operacji (ponieważ funkcja semop() jako argument przyjmuje tablicę)
    struct sembuf operations[1];

    srand(getpid());

    int pizza_type, oven_pizza_quantity, table_pizza_quantity, pizza_from_oven;
    while(1){
        // Losuje typ pizzy [0-9]
        pizza_type = rand() % 10;

        print_time();
        printf(" Przygotowuje pizzę: %d\n", pizza_type);

        // Losuje czas przygotowania pizzy [1-2s]
        sleep_random(1, 2);

        block_access_to_oven(operations, sem_id);

        put_pizza_in_oven(operations, pizza_type, sem_id);

        oven_pizza_quantity = OVEN_CAPACITY - semctl(sem_id, 0, GETVAL);
        print_time();
        printf(" Dodałem pizze: %d Liczba pizz w piecu: %d\n", pizza_type, oven_pizza_quantity);

        unlock_access_to_oven(operations, sem_id);

        // Losuje czas pieczenia [4-5s]
        sleep_random(4, 5);

        block_access_to_oven(operations, sem_id);
        pizza_from_oven = take_out_pizza_of_oven(operations, sem_id);
        unlock_access_to_oven(operations, sem_id);

        put_pizza_on_table(operations, pizza_from_oven, sem_id);

        oven_pizza_quantity = OVEN_CAPACITY - semctl(sem_id, 0, GETVAL);
        table_pizza_quantity = TABLE_CAPACITY - semctl(sem_id, 1, GETVAL);
        print_time();
        printf(" Wyjmuje pizze: %d Liczba pizz w piecu: %d Liczba pizz na stole: %d\n",
               pizza_type, oven_pizza_quantity, table_pizza_quantity);
    }

    return 0;
}
