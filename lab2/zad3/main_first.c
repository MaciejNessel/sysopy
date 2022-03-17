#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>


struct counter{
    int file;
    int dir;
    int char_dev;
    int block_dev;
    int fifo;
    int slink;
    int sock;
}cnt = {0,0,0,0,0,0,0};

char* get_type(struct stat* stat) {
    if (S_ISREG(stat->st_mode)){
        cnt.file++;
        return "file";
    }
    if (S_ISDIR(stat->st_mode)){
        cnt.dir++;
        return "dir";
    }
    if (S_ISCHR(stat->st_mode)){
        cnt.char_dev++;
        return "char dev";
    }
    if (S_ISBLK(stat->st_mode)){
        cnt.block_dev++;
        return "block dev";
    }
    if (S_ISFIFO(stat->st_mode)){
        cnt.fifo++;
        return "fifo";
    }
    if (S_ISLNK(stat->st_mode)){
        cnt.slink++;
        return "slink";
    }
    cnt.sock++;
    return "socket";
}

void print_file_data(char* absolute_path, struct stat file_stat ){
    printf("\nAbsolute path: %s\n", absolute_path);
    printf("Number of hard links: %ld\n", file_stat.st_nlink);
    printf("File type: %s\n", get_type(&file_stat));
    printf("Size: %ld bytes\n", file_stat.st_size);
    printf("Last access: %s", ctime(&file_stat.st_atime));
    printf("Last change: %s", ctime(&file_stat.st_mtime));
}

void print_results(){
    printf("\nProcess successfully  completed. Counter results:\n");
    printf("-> file: %d\n", cnt.file);
    printf("-> dir: %d\n", cnt.dir);
    printf("-> char_dev: %d\n", cnt.char_dev);
    printf("-> block_dev: %d\n", cnt.block_dev);
    printf("-> fifo: %d\n", cnt.fifo);
    printf("-> slink: %d\n", cnt.slink);
    printf("-> sock: %d\n", cnt.sock);
    printf("--> all: %d\n", cnt.file + cnt.dir + cnt.char_dev + cnt.block_dev + cnt.fifo + cnt.slink + cnt.sock);
}

char* get_absolute_path(char* dir_real_path, char* file_path){
    char* absolute_path = calloc(strlen(dir_real_path) + strlen(file_path) + 2, sizeof (char));
    strcat(absolute_path, dir_real_path);
    strcat(absolute_path, "/");
    strcat(absolute_path, file_path);
    return absolute_path;
}

int read_dir(char* dir_path){
    char* dir_real_path = realpath(dir_path, NULL);
    DIR* dir = opendir(dir_path);
    if(!dir){
        printf("Error opening dir\n");
        return 1;
    }

    struct dirent* current;
    struct stat file_stat;
    while ((current = readdir(dir))){
        if (strcmp(current->d_name, "..") == 0 || strcmp(current->d_name, ".") == 0) {
            continue;
        }
        char* absolute_path  = get_absolute_path(dir_real_path, current->d_name );
        lstat(absolute_path, &file_stat);
        print_file_data(absolute_path, file_stat);
        if(S_ISDIR(file_stat.st_mode))
            read_dir(absolute_path);
    }

    free(dir_real_path);
    free(current);
    int close_f = closedir(dir);

    if(close_f != 0){
        printf("Error closing dir\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Wrong number of arguments\n");
        return 0;
    }

    char* dir_path = argv[1];

    if(!read_dir(dir_path)){
        print_results();
    }

    return 0;
}
