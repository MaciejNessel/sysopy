#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int is_whitespace_character(char ch){
    return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\0');
}

int main(int argc, char* argv[]){
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
    char* buff = calloc(actual_line_size, sizeof (char ));

    int can_add = 0;
    int idx = 0;
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

    printf("Process successfully completed.\n");
    return 0;
}