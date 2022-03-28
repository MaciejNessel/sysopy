#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "stdlib.h"
#include "sys/wait.h"
#include "stdbool.h"
#include "string.h"
#include "dirent.h"
#include "sys/stat.h"


int is_text_file(struct dirent* dir){
    return (strstr(dir->d_name, ".txt")) != NULL;
}


int check_file(char* path, char* text){
    FILE* file = fopen(path, "r");
    if(file == NULL){
        printf("Error opening file.");
        return false;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char content[size];
    fread(content, 1, size, file);
    fclose(file);

    if(strstr(content, text)){
        return 1;
    }

    return 0;
}


void check_dir(char* path, int depth, char* text){
    DIR* dirInit = opendir(path);
    struct dirent* dir;
    char newPath[512];
    pid_t child_pid;
    while((dir = readdir(dirInit))!=NULL){
        if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0){
            continue;
        }
        strcpy(newPath, path);
        strcat(newPath, "/");
        strcat(newPath, dir->d_name);

        if(dir->d_type == DT_DIR && depth > 1){
            child_pid = fork();
            if (child_pid == 0) {
                check_dir(newPath, depth - 1, text);
                exit(0);
            }
            else{
                wait(0);
            }
        }
        else if(dir->d_type == DT_REG && is_text_file(dir) && check_file(newPath, text)){
            printf("Relative path: %s \nPID: %d \n", newPath, getpid());
        }
    }
}


int main(int argc, char** argv){
    if(argc != 4){
        printf("Wrong number of arguments.");
        return 0;
    }

    char* dir_path = argv[1];
    char* text = argv[2];
    int depth = atoi(argv[3]);

    check_dir(dir_path, depth, text);

    return 0;
}