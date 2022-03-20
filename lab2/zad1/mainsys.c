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


/* whitespace_characters:
 *  space (' '), tab ('\t'), carriage return ('\r'), newline ('\n'), vertical tab ('\v') and formfeed ('\f')
 */
int is_whitespace_character(char ch){
    return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\0');
}

int main(int argc, char* argv[]){
    //Pomiar czasu
    clock_t clock_start, clock_end;
    struct tms* tms_start = malloc(sizeof (struct tms));
    struct tms* tms_end = malloc(sizeof (struct tms));
    FILE* f = NULL;
    f = fopen("./pomiar_zad_1.txt", "a");


    char* input_file_path = calloc(256, sizeof (char));
    char* output_file_path = calloc(256, sizeof (char));
    if(argc < 3){
        printf("Enter source file path\n");
        scanf("%s", input_file_path);
        printf("Enter output file path\n");
        scanf("%s", output_file_path);
    }
    else if(argc == 3){
        input_file_path = argv[1];
        output_file_path = argv[2];
    } else{
        printf("Too many arguments!\n");
        return 0;
    }
    int input_file = open(input_file_path, O_RDONLY);
    int output_file = open(output_file_path, O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    char c;

    int actual_line_size = 256;
    char* buff = calloc(actual_line_size, sizeof (char));

    int can_add = 0;
    int idx = 0;

    clock_start = times(tms_start);

    while(read(input_file, &c, 1)==1){
        buff[idx] = c;
        if(!is_whitespace_character(c)){
            can_add = 1;
        }
        if(idx+1 == actual_line_size){
            actual_line_size += 256;
            buff = realloc(buff, actual_line_size);
        }

        if(c == 10 && can_add==1){
            write(output_file, buff, idx);
            can_add = 0;
            idx = 0;
        }
        else if(c == 10 && can_add == 0){
            idx = 0;
        }
        else{
            idx++;
        }
    }

    if(idx > 0 && can_add){
        write(output_file, buff, idx);
    }

    clock_end = times(tms_end);
    write_result(clock_start, clock_end, tms_start, tms_end, f);

    printf("Process successfully completed.\n");
    return 0;
}