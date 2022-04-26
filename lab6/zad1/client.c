#include "common.h"

int client_queue = -1;
int server_queue = -1;
int client_id = -1;
pid_t child_pid = -1;
int active = 1;


// Wysy³a komunikat do serwera
void request_send(Message *msg) {
    msg->client_id = client_id;
    if(msgsnd(server_queue, msg, MSG_SIZE, 0) == -1){
        printf("[client][%s] request failed.\n", mtype_str[msg->mtype]);
    }
    printf("[client][%s] sent.\n", mtype_str[msg->mtype]);
}

// Wysy³a komunikat INIT do serwera wraz z wygenerowanym prywatnym kluczem
// Nastêpnie odbiera od serwera identyfikator (client_id)
void register_client(key_t private_key) {
    Message msg;
    msg.mtype = INIT;
    msg.client_id = -1;
    sprintf(msg.message_text, "%d", private_key);
    if (msgsnd(server_queue, &msg, MSG_SIZE, 0) == -1){
        puts("[client][init] INIT request failed.");
        exit(1);
    }
    if (msgrcv(client_queue, &msg, MSG_SIZE, 0, 0) == -1){
        puts("[client][init] catching INIT response failed.");
        exit(1);
    }
    if (sscanf(msg.message_text, "%d", &client_id) < 1){
        puts("[client][init] scanning INIT response failed.");
        exit(1);
    }
    if (client_id < 0){
        puts("[client][init] server cannot have more clients");
        exit(1);
    }
    printf("[client][init] Client ID: %d Queue: %d\n\n", client_id, client_queue);
}

void send2one(Message *msg){
    msg->mtype = _2ONE;

    printf("[client][2one] enter your message: ");
    if (fgets(msg->message_text, 20, stdin) == NULL){
        printf("[client][2one] error reading your command\n");
        return;
    }

    char user[10];
    printf("[client][2one] enter recipient: ");
    if (fgets(user, 10, stdin) == NULL){
        printf("[client][2one] error reading your command\n");
        return;
    }
    msg->recipient = atoi(user);
    request_send(msg);
}

void send2all(Message *msg){
    msg->mtype = _2ALL;

    printf("[client][2all] enter your message: ");
    if (fgets(msg->message_text, 20, stdin) == NULL) {
        printf("[client][2all] error reading your command\n");
        return;
    }

    request_send(msg);
}

void stop_client(){
    Message msg;
    msg.mtype = STOP;
    msg.client_id = client_id;
    sprintf(msg.message_text, "%d", client_id);
    if (msgsnd(server_queue, &msg, MSG_SIZE, 0) == -1){
        puts("[client][stop] SIGINT || STOP request failed.");
        return;
    }
    request_send(&msg);
}

void exit_handler(){
    struct msqid_ds current_state;
    if (msgctl(client_queue, IPC_RMID, &current_state) == -1){
        puts("[client] removing queue failed.");
    }
    puts("[client] queue removed.");
}

void list(Message *msg){
    msg->mtype = LIST;
    request_send(msg);
}


// Obs³uguje sygna³ SIGINT
void sig_handler(){
    puts("[client] Ctrl + C received.");
    stop_client();
}

void handle_command(char* cmd){
    Message msg;
    msg.client_id = client_id;

    if (strcmp(cmd, "STOP") == 0) {
        stop_client();
    } else if (strcmp(cmd, "2ONE") == 0) {
       send2one(&msg);
    } else if (strcmp(cmd, "2ALL") == 0) {
        send2all(&msg);
    } else if (strcmp(cmd, "LIST") == 0) {
        list(&msg);
    } else {
        printf("[client] incorrect command\n");
    }
}

int get_server_queue(char *path, int id_) {
    int key = ftok(path, id_);
    if(key == -1) {
        puts("[client] the key could not be generated.");
        exit(1);
    }
    int queue_id = msgget(key, 0);
    if (queue_id == -1){
        puts("[client] opening queue failed.");
        exit(1);
    }
    return queue_id;
}

key_t setup_queues(){
    char* path = getenv("HOME");
    if(path == NULL){
        printf("[client] getting path failed.");
    }

    server_queue = get_server_queue(path, PROJ);
    if(server_queue == -1){
        puts("[client] getting server queue failed");
        exit(1);
    }

    key_t private_key = ftok(path, getpid());
    if(private_key == -1){
        puts("[client] private key could not be generated.");
        exit(1);
    }

    client_queue = msgget(private_key, IPC_CREAT | IPC_EXCL | 0666);
    if (client_queue == -1){
        puts("[client] create client queue failed.");
        exit(1);
    }
    return private_key;
}

void handle_message(Message* msg){
    switch(msg->mtype){
        case _2ONE:
            printf("\n[>>client] received message from %d: %s", msg->client_id, msg->message_text);
            break;
        case STOP:
            puts("\n[client] STOP received from server.");
            active = 0;
            kill(child_pid, SIGINT);
            exit(0);
        default:
            break;
    }
}

int main(){
    key_t private_key = setup_queues();
    register_client(private_key);

    child_pid = fork();
    if(child_pid == -1){
        puts("[client] fork failed.");
        return -1;
    }
    if (child_pid > 0){
        // Parent process- responsible for receiving messages
        signal(SIGINT, sig_handler);
        atexit(exit_handler);
        puts("[client] waiting for messages...");
        Message buffer;
        while(active){
            if (msgrcv(client_queue, &buffer, MSG_SIZE, 0, 0) < 0){
                puts("[client] Receiving message failed.");
                continue;
            }
            handle_message(&buffer);
        }
    }
    else {
        // Child process - responsible for sending messages to server
        char command[20];
        while(active) {
            printf("[client] enter your request: ");
            if (fgets(command, 20, stdin) == NULL){
                printf("[client] error reading your command\n");
                continue;
            }
            int n = strlen(command);
            if (command[n-1] == '\n') command[n-1] = 0;
            handle_command(command);
        }
    }
    puts("[client] end.");
    return 0;
}
