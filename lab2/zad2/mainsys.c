#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


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

    int input_file = open(input_file_path, O_RDONLY);

    int char_cnt = 0;
    int line_cnt = 0;
    char c;
    while(read(input_file, &c, 1)==1){
        if(c == 10){
            line_cnt++;
        }

        if(c == *input_c){
            char_cnt++;
        }
    }

    printf("*SYS* Process successfully  completed. Results:\n");
    printf("Char counter: %d\n", char_cnt);
    printf("Lines counter: %d\n", line_cnt);

    return 0;
}