#include "common.h"

// Global variables
int server_queue = -1;
int clients_queue[MAX_CLIENTS];
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

void result_to_file(struct Message *msg, FILE *file){
    printf("\nTIME: %s", get_time());
    printf("Client ID: %d\n", msg->client_id);
    printf("Message: %s\n", mtype_str[msg->mtype]);

    fprintf(file, "\nTIME: %s", get_time());
    fprintf(file, "Client ID: %d\n", msg->client_id);
    fprintf(file, "Message: %s\n", mtype_str[msg->mtype]);

}

int init(struct Message *msg){
    key_t client_queue_key;
    if (sscanf(msg->message_text, "%d", &client_queue_key) < 0){
        puts("[server][init] reading client_queue_key failed");
        return 1;
    }

    int client_queue_id = msgget(client_queue_key, 0);
    if (client_queue_id == -1) {
        puts("[server][init] reading client_queue_id failed");
        return 1;
    }

    msg->mtype = INIT;
    if (client_count > MAX_CLIENTS - 1) {
        puts("[server][init] maximum number of clients reached.");
        sprintf(msg->message_text, "%d", -1);
    } else {
        int i=0;
        while (i<MAX_CLIENTS){
            if(clients_queue[i] == -1)
                break;
            i++;
        }
        if(i<MAX_CLIENTS){
            clients_queue[i] = client_queue_id;
            sprintf(msg->message_text, "%d", i);
            client_count++;
        }
    }
    if (msgsnd(client_queue_id, msg, MSG_SIZE, 0) == -1){
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
    if (msgsnd(clients_queue[msg->recipient], msg, MSG_SIZE, 0) == -1){
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
    if(client_id > MAX_CLIENTS){
        puts("[server][stop] wrong client id.");
        return;
    }
    if(clients_queue[client_id] != -1){
        if(msgsnd(clients_queue[client_id], msg, MSG_SIZE, 0) == -1){
            printf("[server][stop] request failed.\n");
        }
        clients_queue[client_id] = -1;
        client_count--;
        printf("[server][stop] client removed [%d]\n", client_id);
    }
}


void sig_handler(){
    puts("\n[server][Ctrl + C received.]\n");
    active = 0;
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
    struct msqid_ds current_state;
    if (msgctl(server_queue, IPC_RMID, &current_state) == -1){
        puts("[server] removing queue failed.");
    }
    puts("[server] queue removed.");
}

void setup_queue(){
    char* path = getenv("HOME");
    if(path == NULL){
        printf("[server] Getting path failed.");
    }

    key_t public_key = ftok(path, PROJ);
    if(public_key == -1){
        puts("[server] Generation of public_key failed.");
        exit(1);
    }

    server_queue = msgget(public_key, IPC_CREAT | IPC_EXCL | 0666);
    if(server_queue == -1){
        puts("[server] Creation of public queue failed");
        exit(1);
    }
}

void handle_server_queue(struct Message *msg, FILE *file){
    puts("\n-------------------------------------------------------\n");
    result_to_file(msg, file);
    if (msg == NULL) return;
    switch(msg->mtype){
        case LIST:
            list();
            break;
        case _2ALL:
            _2all(msg);
            break;
        case _2ONE:
            _2one(msg);
            break;
        case STOP:
            stop_client(msg);
            break;
        case INIT:
            if(init(msg)==1)
                puts("[server] init client failed.");
            break;
        default:
            break;
    }
}

int main(){
    setup_queue();
    atexit(exit_handler);
    signal(SIGINT, sig_handler);

    for(int i=0; i<MAX_CLIENTS; i++){
        clients_queue[i] = -1;
    }

    FILE *file = fopen("./logs", "a");
    puts("[server] working...");
    Message buffer;
    while (active){
        if (msgrcv(server_queue, &buffer, MSG_SIZE, 0, 0) < 0){
            puts("[server] receiving message failed.");
            continue;
        }
        handle_server_queue(&buffer, file);
    }

    fclose(file);
    return 0;
}