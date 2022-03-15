#include <stdio.h>
#include "./btm.h"
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include "string.h"

double time_diff(clock_t start, clock_t end){
    return (double)(end - start) / sysconf(_SC_CLK_TCK);
}

void write_result(clock_t start, clock_t end, struct tms* t_start, struct tms* t_end, FILE* f, char* result){
    printf("%s real_time: %fl\n",result, time_diff(start, end));
    //tms_utime – czas cpu wykonywania procesu w trybie użytkownika
    printf("%s user_time: %fl\n",result,  time_diff(t_start->tms_utime, t_end->tms_utime));
    //tms_stime – czas cpu wykonywania procesu w trybie jądra
    printf("%s system_time: %fl\n",result, time_diff(t_start->tms_stime, t_end->tms_stime));

    fprintf(f, "%s real_time: %fl\n",result, time_diff(start, end));
    fprintf(f, "%s user_time: %fl\n",result, time_diff(t_start->tms_utime, t_end->tms_utime));
    fprintf(f, "%s system_time: %fl\n", result, time_diff(t_start->tms_stime, t_end->tms_stime));
}

int main(int argc, char **argv){
    FILE* f = NULL;
    struct block_array* blockArray= NULL;

    struct tms* tms_start = malloc(sizeof (struct tms));
    struct tms* tms_end = malloc(sizeof (struct tms));
    clock_t clock_start, clock_end, total_clock_start, total_clock_end;

    f = fopen("./raport2.txt", "a");

    char* result = "";
    total_clock_start = times(tms_start);
    for(int i = 1; i<argc; i++){
        clock_start = times(tms_start);
        if(strcmp(argv[i], "create_table") == 0){
            result = "create_table ";
            i++;
            blockArray = create_table(atoi(argv[i]));
        }
        else if(strcmp(argv[i], "wc_files") == 0){
            result = "wc_files ";
            i++;
            char* command = calloc(100, sizeof(char));

            while (strcmp(argv[i], "wc_files") && strcmp(argv[i], "create_table") && strcmp(argv[i], "remove_block") && argv[i]!=NULL){
                strcat(command, argv[i]);
                strcat(command, " ");
                i++;
                if(i+1>argc){
                    break;
                }

            }
            wc_files(command);
            add_temp_to_array(blockArray);
            free(command);
            i--;
        }
        else if(strcmp(argv[i], "remove_block") == 0){
            result = "remove_block ";
            i++;
            if(blockArray!=NULL){
                delete_block(atoi(argv[i]), blockArray);
            }
            else{
                printf("Tablica musi zostac najpierw zadeklarowana");
                exit(0);
            }
        }
        else{
            result = "wrong_arg ";
            printf("Podano bledny argument %s\n", argv[i]);
            break;
        }

        clock_end = times(tms_end);
        write_result(clock_start, clock_end, tms_start, tms_end, f, result);

    }
    total_clock_end = times(tms_end);
    write_result(total_clock_start, total_clock_end, tms_start, tms_end, f, "TOTAL :");

    fclose(f);
    free(blockArray);
    return 0;
}