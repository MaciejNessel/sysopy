#include <stdio.h>
#include <stdlib.h>


/* whitespace_characters:
 *  space (' '), tab ('\t'), carriage return ('\r'), newline ('\n'), vertical tab ('\v') and formfeed ('\f')
 */
int is_whitespace_character(char ch){
    return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\0');
}

int is_empty_line(char* s){
    do {
        if(!is_whitespace_character(*(s++)))
            return 0;
    } while (*s != '\0');

    return 1;
}

int main(int argc, char *argv[]){
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

    FILE* input_file = fopen(input_file_path, "r");
    FILE* output_file = fopen(output_file_path, "w");
    if(input_file == NULL || output_file == NULL){
        printf("Reading files error.\n");
        return 0;
    }

    char buff[1000];
    while (fgets(buff, 1000, input_file)){
        if (!is_empty_line(buff)){
            fputs(buff, output_file);
        }
    }

    printf("Process successfully  completed.\n");

    fclose(input_file);
    fclose(output_file);

    return 0;

}