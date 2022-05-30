#include <stdio.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define ELVES 10
#define REINDEERS 9
#define ELVES_GROUP 3
#define GIFTS_TO_DELIVER 3

int no_reindeer_waiting = 0;
int no_elves_waiting = 0;
int elves_waiting_id[3];
int is_santa_sleeping = 1;
int delivered_gifts = 0;

pthread_t* workers_threads[ELVES+REINDEERS];

pthread_mutex_t reindeer_waiting_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elves_waiting_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_sleeping_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t santa_sleep_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_can_help_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_can_deliver_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t need_santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_waiting_cond = PTHREAD_COND_INITIALIZER;


void santa_work();
void elf_work(int* id);
void reindeer_work(int* id);

int main() {
    pthread_t santa;

    int* reindeer_id = malloc(sizeof(int)*REINDEERS);
    int* elf_id = malloc(sizeof(int)*ELVES);

    pthread_create(&santa, NULL, (void *(*)(void *))santa_work, NULL);

    for (int i = 0; i < ELVES; i++) {
        elf_id[i] = i;
        pthread_create(&workers_threads[i], NULL, (void *(*)(void *)) elf_work, &elf_id[i]);
    }

    for (int i = ELVES; i < ELVES+REINDEERS; i++) {
        reindeer_id[i] = i;
        pthread_create(&workers_threads[i], NULL, (void *(*)(void *)) reindeer_work, &reindeer_id[i]);
    }

    for (int i = 0; i < ELVES + REINDEERS; i++) {
        pthread_join(workers_threads[i], NULL);
    }

    pthread_join(santa, NULL);

    free(elf_id);
    free(reindeer_id);

    pthread_cond_destroy(&need_santa_cond);
    pthread_cond_destroy(&elves_waiting_cond);
    pthread_cond_destroy(&santa_can_help_cond);
    pthread_cond_destroy(&santa_sleep_cond);
    pthread_cond_destroy(&santa_can_deliver_cond);

    pthread_mutex_destroy(&elves_waiting_mutex);
    pthread_mutex_destroy(&santa_sleeping_mutex);
    pthread_mutex_destroy(&reindeer_waiting_mutex);

    return 0;
}

void santa_work(){
    while (1){
        pthread_mutex_lock(&santa_sleeping_mutex);
        is_santa_sleeping = 1;
        printf("Mikołaj: zasypiam\n");

        // wznawia wątki które oczekiwały na mikołaja
        pthread_cond_broadcast(&santa_sleep_cond);

        // oczekuje na wezwania od reniferów albo elfów
        pthread_cond_wait(&need_santa_cond,&santa_sleeping_mutex);

        puts("Mikołaj: budzę się");
        is_santa_sleeping = 0;
        pthread_mutex_unlock(&santa_sleeping_mutex);

        // w pierwszej kolejności sprawdza czy jest możliwość rozwiezienia prezentów
        pthread_mutex_lock(&reindeer_waiting_mutex);
        if(no_reindeer_waiting >= REINDEERS){
            // wznawia wątki odpowiedzialne za pracę reniferów
            pthread_cond_broadcast(&santa_can_deliver_cond);
            pthread_mutex_unlock(&reindeer_waiting_mutex);
            puts("Mikolaj: dostarczam zabawki");
            sleep(rand() % 3 + 2);
            delivered_gifts++;
        }
        else{
            pthread_mutex_unlock(&reindeer_waiting_mutex);
            pthread_mutex_lock(&elves_waiting_mutex);
            if (no_elves_waiting >= ELVES_GROUP) {
                // rozwiązuje problemy obecnych elfów
                pthread_cond_broadcast(&santa_can_help_cond);
                printf("Mikołaj: rozwiązuje problemy elfów [%d, %d, %d]\n", elves_waiting_id[0], elves_waiting_id[1], elves_waiting_id[2]);
                sleep(rand() % 2 + 1);

                // umożliwia kolejnym elfom czekanie na mikołaja
                no_elves_waiting = 0;
                pthread_cond_broadcast(&elves_waiting_cond);
            }
            pthread_mutex_unlock(&elves_waiting_mutex);
        }

        // kończy pracę mikołaja i jego pracowników jeśli dostarczył już wszystkie prezenty
        if(delivered_gifts == GIFTS_TO_DELIVER){
            for (int i = 0; i < ELVES + REINDEERS; i++) {
                if(pthread_cancel(workers_threads[i])) {
                    perror("");
                }
            }
            printf("Koniec pracy Mikołaja.\n");
            break;
        }
    }
    pthread_exit((void *)0);
}

void reindeer_work(int* id){
    while(1){
        sleep((rand() % 6) + 5);
        pthread_mutex_lock(&reindeer_waiting_mutex);
        no_reindeer_waiting++;
        printf("Renifer: czeka %d reniferow na Mikolaja [ID: %d]\n", no_reindeer_waiting, *id);

        // gdy wszystkie renifery wrócą z wakacji powiadamia mikołaja
        if(no_reindeer_waiting == REINDEERS){
            pthread_mutex_unlock(&reindeer_waiting_mutex);
            pthread_mutex_lock(&santa_sleeping_mutex);
            if (is_santa_sleeping == 0) pthread_cond_wait(&santa_sleep_cond, &santa_sleeping_mutex);

            printf("Renifer: wybudzam Mikolaja [ID: %d] \n", *id);
            pthread_cond_broadcast(&need_santa_cond);
            pthread_mutex_unlock(&santa_sleeping_mutex);
            pthread_mutex_lock(&reindeer_waiting_mutex);
        }

        // oczekuje na mikołaja, aby dostarczyć prezenty
        pthread_cond_wait(&santa_can_deliver_cond, &reindeer_waiting_mutex);
        no_reindeer_waiting--;
        pthread_mutex_unlock(&reindeer_waiting_mutex);
        sleep(rand()%3 +2);
    }
}

void elf_work(int* id){
    while (1){
        sleep(rand() % 4 + 2);
        pthread_mutex_lock(&elves_waiting_mutex);

        // zabezpiecza, żeby więcej niż 3 elfy nie czekały na mikołaja
        while(no_elves_waiting >=3){
            printf("Elf: czeka na powrót elfów [ID: %d] \n",*id);
            pthread_cond_wait(&elves_waiting_cond, &elves_waiting_mutex);
        }
        if(no_elves_waiting<3){
            elves_waiting_id[no_elves_waiting] = *id;
            no_elves_waiting++;
            printf("Elf: czeka %d elfów na Mikołaja [ID: %d] \n", no_elves_waiting,*id);

            // grupa 3 elfów oczekuje na mikołaja
            if(no_elves_waiting == 3){
                pthread_mutex_unlock(&elves_waiting_mutex);
                pthread_mutex_lock(&santa_sleeping_mutex);
                pthread_mutex_lock(&reindeer_waiting_mutex);

                // oczekuje, aż mikołaj zakończy obecną prace, a renifery zostaną obsłużone pierwsze
                while(is_santa_sleeping == 0 || no_reindeer_waiting == REINDEERS){
                    pthread_mutex_unlock(&reindeer_waiting_mutex);
                    pthread_cond_wait(&santa_sleep_cond, &santa_sleeping_mutex);
                    pthread_mutex_lock(&reindeer_waiting_mutex);
                }

                printf("Elf: wybudzam Mikołaja [ID: %d] \n", *id);
                pthread_cond_broadcast(&need_santa_cond);
                pthread_mutex_lock(&elves_waiting_mutex);
                pthread_mutex_unlock(&santa_sleeping_mutex);
                pthread_mutex_unlock(&reindeer_waiting_mutex);
            }
            pthread_cond_wait(&santa_can_help_cond, &elves_waiting_mutex);
            pthread_mutex_unlock(&elves_waiting_mutex);
        }
    }
}



