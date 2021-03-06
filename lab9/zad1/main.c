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
        printf("Miko??aj: zasypiam\n");

        // wznawia w??tki kt??re oczekiwa??y na miko??aja
        pthread_cond_broadcast(&santa_sleep_cond);

        // oczekuje na wezwania od renifer??w albo elf??w
        pthread_cond_wait(&need_santa_cond,&santa_sleeping_mutex);

        puts("Miko??aj: budz?? si??");
        is_santa_sleeping = 0;
        pthread_mutex_unlock(&santa_sleeping_mutex);

        // w pierwszej kolejno??ci sprawdza czy jest mo??liwo???? rozwiezienia prezent??w
        pthread_mutex_lock(&reindeer_waiting_mutex);
        if(no_reindeer_waiting >= REINDEERS){
            // wznawia w??tki odpowiedzialne za prac?? renifer??w
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
                // rozwi??zuje problemy obecnych elf??w
                pthread_cond_broadcast(&santa_can_help_cond);
                printf("Miko??aj: rozwi??zuje problemy elf??w [%d, %d, %d]\n", elves_waiting_id[0], elves_waiting_id[1], elves_waiting_id[2]);
                sleep(rand() % 2 + 1);

                // umo??liwia kolejnym elfom czekanie na miko??aja
                no_elves_waiting = 0;
                pthread_cond_broadcast(&elves_waiting_cond);
            }
            pthread_mutex_unlock(&elves_waiting_mutex);
        }

        // ko??czy prac?? miko??aja i jego pracownik??w je??li dostarczy?? ju?? wszystkie prezenty
        if(delivered_gifts == GIFTS_TO_DELIVER){
            for (int i = 0; i < ELVES + REINDEERS; i++) {
                if(pthread_cancel(workers_threads[i])) {
                    perror("");
                }
            }
            printf("Koniec pracy Miko??aja.\n");
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

        // gdy wszystkie renifery wr??c?? z wakacji powiadamia miko??aja
        if(no_reindeer_waiting == REINDEERS){
            pthread_mutex_unlock(&reindeer_waiting_mutex);
            pthread_mutex_lock(&santa_sleeping_mutex);
            if (is_santa_sleeping == 0) pthread_cond_wait(&santa_sleep_cond, &santa_sleeping_mutex);

            printf("Renifer: wybudzam Mikolaja [ID: %d] \n", *id);
            pthread_cond_broadcast(&need_santa_cond);
            pthread_mutex_unlock(&santa_sleeping_mutex);
            pthread_mutex_lock(&reindeer_waiting_mutex);
        }

        // oczekuje na miko??aja, aby dostarczy?? prezenty
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

        // zabezpiecza, ??eby wi??cej ni?? 3 elfy nie czeka??y na miko??aja
        while(no_elves_waiting >=3){
            printf("Elf: czeka na powr??t elf??w [ID: %d] \n",*id);
            pthread_cond_wait(&elves_waiting_cond, &elves_waiting_mutex);
        }
        if(no_elves_waiting<3){
            elves_waiting_id[no_elves_waiting] = *id;
            no_elves_waiting++;
            printf("Elf: czeka %d elf??w na Miko??aja [ID: %d] \n", no_elves_waiting,*id);

            // grupa 3 elf??w oczekuje na miko??aja
            if(no_elves_waiting == 3){
                pthread_mutex_unlock(&elves_waiting_mutex);
                pthread_mutex_lock(&santa_sleeping_mutex);
                pthread_mutex_lock(&reindeer_waiting_mutex);

                // oczekuje, a?? miko??aj zako??czy obecn?? prace, a renifery zostan?? obs??u??one pierwsze
                while(is_santa_sleeping == 0 || no_reindeer_waiting == REINDEERS){
                    pthread_mutex_unlock(&reindeer_waiting_mutex);
                    pthread_cond_wait(&santa_sleep_cond, &santa_sleeping_mutex);
                    pthread_mutex_lock(&reindeer_waiting_mutex);
                }

                printf("Elf: wybudzam Miko??aja [ID: %d] \n", *id);
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



