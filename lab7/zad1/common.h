#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/time.h>

#define OVEN_CAPACITY 5
#define TABLE_CAPACITY 5
#define MAX_NO_WORKERS 50

#define TABLE_KEY ftok(getenv("HOME"), 't')
#define OVEN_KEY ftok(getenv("HOME"), 'o')
#define SEM_KEY ftok(getenv("HOME"), 's')


struct pizza_place{
    int types_of_pizza[OVEN_CAPACITY];
    int put_on;
    int take_from;
};

void print_time(){
    struct timeval tp;
    gettimeofday(&tp, 0);
    time_t curtime = tp.tv_sec;
    struct tm *t = localtime(&curtime);
    printf("%d %d:%d:%d:%ld",getpid(), t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
}

void sleep_random(int min, int max){
    int wait_time = min + rand() % (max-min+1);
    sleep(wait_time);
}

#endif