#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]){
    if (argc != 5){
        printf("Wrong number of arguments - producer!");
        exit(1);
    }

    char* pipe_path = argv[1];
    int row = atoi(argv[2]);
    char* file_path = argv[3];
    int N = atoi(argv[4]);

    FILE* pipe = fopen(pipe_path, "w");
    if(pipe < 0) {
        printf("Pipe error!\n");
    }

    FILE* file = fopen(file_path, "r");
    if(file < 0) {
        printf("File opening error!\n");
    }

    char buffer[N];
    char text[N + 5];

    while(fread(buffer, sizeof(char), N, file) == N){
        sleep(1);
        buffer[strlen(buffer) - 1] = '\n';
        snprintf(text, sizeof(text), "%d:%s", row, buffer);
        printf("> Producer write to pipe: %s\n", text);
        fwrite(text, sizeof(char), N + 5, pipe);
    }

    fclose(pipe);
    fclose(file);

    return 0;
}