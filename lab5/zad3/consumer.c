#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#define MAX_LINE_LEN 256

void save_to_file(FILE* file, char* text, int row, char* file_path){
    rewind(file);
    int fd = fileno(file);
    char buffer[MAX_LINE_LEN+1];
    int row_counter = 0;

    // Ponieważ, w tym przypadku kilka procesów będzie zapisywać do jednego, wspólnego pliku, dlatego należy użyć funkcji flock()
    flock(fd, LOCK_EX);
    while (fgets(buffer, MAX_LINE_LEN+1, file) != NULL){
        if (row_counter == row) {
            // Dopisujemy do istniejącego ciągu znaków w danej linii
            int i=0;
            while (i<MAX_LINE_LEN && buffer[i]!=' '){
                i++;
            }
            fseek(file, row * MAX_LINE_LEN + i, SEEK_SET);
            fprintf(file, text, strlen(text) + 1);
            fflush(file);
            break;
        }
        row_counter++;
    }
    flock(fd, LOCK_UN);
}

int main(int argc, char* argv[]){
    if (argc != 4){
        printf("Wrong number of arguments - consumer!");
        exit(1);
    }

    char* pipe_path = argv[1];
    FILE* pipe = fopen(pipe_path, "r");
    if(pipe < 0) {
        printf("Pipe error %s!\n", pipe_path);
    }

    char* file_path = argv[2];
    FILE* file = fopen(file_path, "r+");
    if(file < 0) {
        printf("File open error %s\n", file_path);
    }

    int N = atoi(argv[3]);
    char buffer[N];

    while(fread(buffer, sizeof(char), N, pipe) == N){
        printf("> Consumer read from pipe: %s", buffer);

        // Rozdzielamy numer wiersza oraz fragment tekstu (format zapisu buffer to "nr_wiersza:text")
        char* seq = strtok(buffer, ":");
        int num = atoi(seq);
        seq = strtok(NULL, "\n");

        // Umieszczamy odczytane znaki w pliku z wynikami
        save_to_file(file, seq, num, file_path);
    }

    fclose(pipe);
    fclose(file);

    return 0;
}
