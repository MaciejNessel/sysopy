#include <unistd.h>
#include <wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MAX_LINE_LEN 256


char* create_empty_line(){
    char* line = calloc(MAX_LINE_LEN, sizeof(char));
    for (int i = 0; i < MAX_LINE_LEN-1; i++){
        line[i] = ' ';
    }
    line[MAX_LINE_LEN-1] = '\n';
    return line;
}

void create_empty_file(char* filename, int rows){
    FILE* file = fopen(filename, "w");
    char* line = create_empty_line();
    for (int i = 0; i < rows - 1; i++){
        fprintf(file, "%s", line);
    }
    line[MAX_LINE_LEN-1] = '\0';
    fprintf(file, "%s", line);
    fclose(file);
    free(line);
}


void start(char* result, int producers, int consumers){
    char *cons_N = "10";
    char *prod_N = "5";
    char* input_files[] = {"files/producer/A.txt", "files/producer/B.txt", "files/producer/C.txt", "files/producer/D.txt", "files/producer/E.txt"};
    create_empty_file(result, producers);

    for (int i = 0; i < consumers; i++){
        if (fork() == 0){
            char* exec_argv[]={"./consumer", "/tmp/myfifo", result, cons_N, NULL};
            execvp("./consumer", exec_argv);
        }
    }

    for (int i = 0; i < producers; i++){
        if (fork() == 0){
            char row[2];
            sprintf(row, "%d", i);
            char* exec_argv[]={"./producer", "/tmp/myfifo", row, input_files[i], prod_N, NULL};
            execvp("./producer", exec_argv);
        }
    }

    for (int i = 0; i < producers + consumers; i++)
        wait(NULL);

}


int main(){
    // Tworzy nowy potok nazwany "pipe"
    mkfifo("/tmp/myfifo", 0666);

    // Sprawdza przypadki podane w zadaniu
    puts(">> 1. wielu producentów, jeden konsument   <<");
    start("files/consumer/1.txt", 5, 1);

    puts(">> 2. jeden producent, wielu konsumentów   <<");
    start("files/consumer/2.txt", 1, 5);

    puts(">> 3. wielu producentów, wielu konsumentów <<");
    start("files/consumer/3.txt", 5, 5);

    return 0;
}

