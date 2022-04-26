#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define QUEUE_NAME "/SERVER"
#define MAX_CLIENTS  10
#define MAX_MESSAGE_SIZE 1024

typedef enum mtype {
    STOP = 1, LIST = 2, _2ALL = 3, _2ONE = 4, INIT = 5
} mtype;

const char * const mtype_str[] =
{
        [STOP] = "STOP",
        [LIST] = "LIST",
        [_2ALL]  = "2ALL",
        [_2ONE]  = "2ONE",
        [INIT]  = "INIT"
};

typedef struct Message {
    long mtype;
    int client_id;
    char message_text[MAX_MESSAGE_SIZE];
    int recipient;
} Message;

const size_t MSG_SIZE = sizeof(Message);

#endif