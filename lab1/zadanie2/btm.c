#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "btm.h"


struct block_array* create_table(int size){
    if(size<1){
        printf("The size is too small");
        exit(1);
    }

    struct block_array *result = calloc(1, sizeof(struct block_array));

    char** arr = (char**) calloc(size, sizeof(char*));
    result->array = arr;
    result->no_blocks = 0;

    return result;
}


void wc_files(char* files){
    if(files==NULL){
        printf("File path error\n");
        exit(1);
    }
    char* command = calloc(100, sizeof(char));
    strcat(command, "wc ");
    strcat(command, files);
    strcat(command, "> ./temp ");
    int systemRet = system(command);
    if(systemRet == -1){
        printf("System method failed");
        exit(1);
    }
    free(command);
}


char* load_from_tmp(){
    FILE *f = NULL;
    f = fopen("./temp", "r");

    if (f == NULL){
        printf("Temp file error");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    int size = ftell(f);

    fseek(f, 0, SEEK_SET);
    char* block = calloc(size, sizeof(char));
    char* temp = block;
    char znak = ' ';
    while(znak != EOF){
        znak = fgetc(f);
        *temp = znak;
        temp++;
    }
    fclose(f);
    int systemRet = system("rm temp");
    if(systemRet == -1){
        printf("System method failed");
    }
    return block;
}


void add_temp_to_array(struct block_array *blockArray){
    char* new_block;
    new_block = load_from_tmp();
    blockArray->array[blockArray->no_blocks] = new_block;
    blockArray->no_blocks++;
}


void delete_block(int id, struct block_array *block_array) {
    if(id>=block_array->no_blocks){
        printf("No block with this id\nNothing was removed");
        return;
    }
    free(block_array->array[id]);
    block_array->array[id] = NULL;
}