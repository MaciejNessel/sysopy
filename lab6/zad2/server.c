#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>

#include "common.h"

// Global variables
mqd_t server_queue = -1;
mqd_t clients_queue[MAX_CLIENTS];
int active = 1;
int client_count = 0;


char* get_time(){
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    char* result =  asctime(timeinfo);
    return result;
}

void result_to_file(Message *msg, FILE *file){
    printf("\nTIME: %s", get_time());
    printf("Client ID: %d\n", msg->client_id);
    printf("Message: %s\n", mtype_str[msg->mtype]);

    fprintf(file, "\nTIME: %s", get_time());
    fprintf(file, "Client ID: %d\n", msg->client_id);
    fprintf(file, "Message: %s\n", mtype_str[msg->mtype]);
}

int init(struct Message *msg){
    char client_queue_key[20];
    if (sscanf(msg->message_text, "%s", client_queue_key) < 0){
        puts("[server][init] reading client_queue_key failed");
        return 1;
    }

    mqd_t client_queue = mq_open(client_queue_key, O_RDWR);
    if (client_queue == -1) {
        puts("[server][init] reading client_queue failed");
        return 1;
    }

    msg->mtype = INIT;
    if (client_count > MAX_CLIENTS - 1) {
        puts("[server][init] maximum number of clients reached.");
        sprintf(msg->message_text, "%d", -1);
    }
    else {
        int i = 0;
        while (i < MAX_CLIENTS){
            if(clients_queue[i] == -1)
                break;
            i++;
        }
        if(i < MAX_CLIENTS){
            clients_queue[i] = client_queue;
            sprintf(msg->message_text, "%d", i);
            client_count++;
        }
    }
    if (mq_send(client_queue, (char *) msg, MSG_SIZE, msg->mtype) == -1){
        puts("[server][init] INIT response failed");
        return 1;
    }
    return 0;
}

void list(){
    printf("[server][list] No. clients: %d\n", client_count);
    for (int i=0; i<client_count; i++){
        printf("> Client ID: %d (queue: %d)\n", i, clients_queue[i]);
    }
}

void _2one(struct Message *msg){
    msg->mtype = _2ONE;
    if(msg->recipient>MAX_CLIENTS){
        puts("[server][2one] wrong client id.");
        return;
    }

    if (mq_send(clients_queue[msg->recipient], (char *) msg, MSG_SIZE, msg->mtype) == -1){
        puts("[server][2one] sending message failed");
        return;
    }
    printf("[server][2one] sent to %d\n", msg->recipient);
}

void _2all(struct Message *msg){
    puts("[server][2all] started sending...");
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_queue[i] != -1){
            msg->recipient = i;
            _2one(msg);
        }
    }
    puts("[server][2all] ended.");
}

void stop_client(struct Message *msg){
    int client_id = msg->client_id;
    msg->mtype = STOP;
    if(client_id > MAX_CLIENTS){
        puts("[server][stop] wrong client id.");
        return;
    }
    if(clients_queue[client_id] != -1){
        if (mq_send(clients_queue[client_id], (char *) msg, MSG_SIZE, msg->mtype) == -1){
            printf("[server][stop] request failed.\n");
        }
        if(mq_close(clients_queue[client_id]) == -1){
            puts("[server] closing client queue failed.");
        }
        clients_queue[client_id] = -1;
        client_count--;
        printf("[server][stop] client removed [%d]\n", client_id);
    }
}

void sig_handler(){
    puts("\n[server][Ctrl + C received.]\n");
    active = 0;
    exit(0);
}

void exit_handler(){
    //Delete client's queues and send STOP to them
    Message msg;
    msg.mtype = STOP;
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clients_queue[i]!=-1){
            msg.client_id = i;
            stop_client(&msg);
        }
    }
    // Delete server queue
    if(mq_close(server_queue) == -1){
        puts("[server] closing server queue failed.");
        return;
    }
    if(mq_unlink(QUEUE_NAME) == -1){
        puts("[server] removing server queue failed.");
        return;
    }
    puts("[server] queue removed.");
}

void setup_queue(const char* name){
    mq_unlink(name);
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_CLIENTS;
    attr.mq_msgsize = MSG_SIZE;
    if((server_queue = mq_open(name, O_RDWR | O_CREAT, 0666, &attr)) == -1){
        puts("[server] open server queue failed.");
        exit(1);
    }
}

void handle_server_queue(struct Message *msg, FILE *file){
    puts("\n-------------------------------------------------------\n");
    if (msg == NULL) return;
    switch(msg->mtype){
        case LIST:
            result_to_file(msg, file);
            list();
            break;
        case _2ALL:
            result_to_file(msg, file);
            _2all(msg);
            break;
        case _2ONE:
            result_to_file(msg, file);
            _2one(msg);
            break;
        case STOP:
            result_to_file(msg, file);
            stop_client(msg);
            break;
        case INIT:
            result_to_file(msg, file);
            if(init(msg)==1)
                puts("[server] init client failed.");
            break;
        default:
            break;
    }
}

int main(){
    setup_queue(QUEUE_NAME);
    atexit(exit_handler);
    signal(SIGINT, sig_handler);

    for(int i=0; i<MAX_CLIENTS; i++){
        clients_queue[i] = -1;
    }

    FILE *file = fopen("./logs", "a");
    puts("[server] working...");
    Message buffer;
    while (active){
        if(mq_receive(server_queue, (char*) &buffer, MSG_SIZE, NULL) == -1){
            puts("[server] receiving  message failed.");
            continue;
        }
        handle_server_queue(&buffer, file);
    }

    fclose(file);
    return 0;
}