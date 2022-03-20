#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>

/*
 * Pomiar czasu
 */
double time_diff(clock_t start, clock_t end){
    return (double)(end - start) / sysconf(_SC_CLK_TCK);
}
void write_result(clock_t start, clock_t end, struct tms* t_start, struct tms* t_end, FILE* f){
    printf("Pomiar przy użyciu funkcji systemowych:\n");
    printf("real_time: %fl\n", time_diff(start, end));
    //tms_utime – czas cpu wykonywania procesu w trybie użytkownika
    printf("user_time: %fl\n", time_diff(t_start->tms_utime, t_end->tms_utime));
    //tms_stime – czas cpu wykonywania procesu w trybie jądra
    printf("system_time: %fl\n", time_diff(t_start->tms_stime, t_end->tms_stime));

    fprintf(f, "Pomiar przy użyciu funkcji systemowych:\n");
    fprintf(f, "real_time: %fl\n", time_diff(start, end));
    fprintf(f, "user_time: %fl\n", time_diff(t_start->tms_utime, t_end->tms_utime));
    fprintf(f, "system_time: %fl\n", time_diff(t_start->tms_stime, t_end->tms_stime));
}

int main(int argc, char* argv[]){
    char* input_c;
    char* input_file_path = calloc(256, sizeof (char));
    if(argc < 3){
        printf("Required arguments were not provided\n");
        return 0;
    }
    else if(argc == 3){
        if(strlen(argv[1]) != 1){
            printf("Entered an invalid character\n");
            return 0;
        }
        input_file_path = argv[2];
        input_c = argv[1];
    }else{
        printf("Too many arguments!\n");
        return 0;
    }

    //Pomiar czasu
    clock_t clock_start, clock_end;
    struct tms* tms_start = malloc(sizeof (struct tms));
    struct tms* tms_end = malloc(sizeof (struct tms));
    FILE* f = NULL;
    f = fopen("./pomiar_zad_2.txt", "a");


    int input_file = open(input_file_path, O_RDONLY);

    int char_cnt = 0;
    int line_cnt = 0;
    char c;

    clock_start = times(tms_start);

    while(read(input_file, &c, 1)==1){
        if(c == 10){
            line_cnt++;
        }

        if(c == *input_c){
            char_cnt++;
        }
    }

    clock_end = times(tms_end);
    write_result(clock_start, clock_end, tms_start, tms_end, f);

    printf("*SYS* Process successfully  completed. Results:\n");
    printf("Char counter: %d\n", char_cnt);
    printf("Lines counter: %d\n", line_cnt);

    return 0;
}