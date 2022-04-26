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

int client_queue = -1;
int server_queue = -1;
int client_id = -1;
pid_t child_pid = -1;
int active = 1;
char client_queue_name[20];


void request_send(Message *msg) {
    msg->client_id = client_id;
    if (mq_send(server_queue, (char *) msg, MSG_SIZE, msg->mtype) == -1){
        printf("[client][%s] request failed.\n", mtype_str[msg->mtype]);
    }
    printf("[client][%s] sent.\n", mtype_str[msg->mtype]);
}

void register_client() {
    Message msg;
    msg.mtype = INIT;
    msg.client_id = -1;
    sprintf(msg.message_text,"%s", client_queue_name);
    if (mq_send(server_queue, (char *) &msg, MSG_SIZE, msg.mtype) == -1){
        puts("[client][init] INIT request failed.");
        exit(1);
    }
    Message received_msg;
    if (mq_receive(client_queue, (char*) &received_msg, MSG_SIZE, NULL) == -1){
        puts("[client][init] catching INIT response failed.");
        exit(1);
    }
    client_id = atoi(received_msg.message_text);
    if (client_id < 0){
        puts("[client][init] server cannot have more clients");
        exit(1);
    }
    printf("[client][init] Client ID: %d Queue: %d\n\n", client_id, client_queue);
}

void list(Message *msg){
    msg->mtype = LIST;
    request_send(msg);
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
    if (mq_send(server_queue, (char *) &msg, MSG_SIZE, msg.mtype) == -1){
        puts("[client][stop] SIGINT || STOP request failed.");
        exit(1);
    }
    request_send(&msg);
}

void exit_handler(){
    if(mq_close(server_queue) == -1){
        puts("[client] closing server queue failed");
    }
    if(mq_close(client_queue) == -1){
        puts("[client] closing client queue failed");
    }
    if(mq_unlink(client_queue_name) == -1){
        puts("[client] closing client queue failed");
    }
    puts("[client] queues closed, removed.");
}

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

void setup_queue(){
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_CLIENTS;
    attr.mq_msgsize = MSG_SIZE;

    if((client_queue = mq_open(client_queue_name, O_RDWR | O_CREAT, 0666, &attr)) == -1){
        puts("[client] create client queue failed");
        exit(1);
    }
    if ((server_queue = mq_open(QUEUE_NAME, O_RDWR)) == -1){
        puts("[client] opening server queue failed.");
        exit(1);
    }
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
    sprintf(client_queue_name, "/%d", getpid());
    setup_queue();
    register_client();

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
            if (mq_receive(client_queue, (char*) &buffer, MSG_SIZE, NULL) == -1){
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
