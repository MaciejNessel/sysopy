#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char *argv[]){
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

    FILE* input_file = fopen(input_file_path, "r");
    if(!input_file){
        printf("Reading file error.\n");
        return 0;
    }

    char* c = calloc(2, sizeof (char));
    int char_cnt = 0;
    int line_cnt = 0;
    while (fgets(c, 2, input_file)){
        if(*c == 10){
            line_cnt++;
        }

        if(*c == *input_c){
            char_cnt++;
        }
    }
    fclose(input_file);
    printf("*LIB* Process successfully  completed. Results:\n");
    printf("Char counter: %d\n", char_cnt);
    printf("Lines counter: %d\n", line_cnt);

    return 0;

}