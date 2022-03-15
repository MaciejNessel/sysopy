#ifndef LAB1_BTM_H
#define LAB1_BTM_H

struct block_array{
    char** array;
    int no_blocks;
};

typedef struct block_array block_array;

struct block_array* create_table(int size);
void wc_files(char* files);
char* load_from_tmp();
void add_temp_to_array(struct block_array* array);
void delete_block(int id, struct block_array *array);

#endif //LAB1_BTM_H
