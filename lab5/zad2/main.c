#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void get_mails(char* mode) {
    printf("Maile posortowane według: %s\n", mode);

    char* command;
    if (!strcmp(mode,"data")) {
        command = "echo | mail -H";
    } else if (!strcmp(mode, "nadawca")) {
        command = "echo | mail -H | sort -k 3";
    } else {
        puts("Wrong sort mode.");
        exit(1);
    }

    FILE* file = popen(command, "r");
    if (file == NULL){
        printf("Error popen.");
        exit(1);
    }

    char *line;
    size_t len = 0;
    while((getline(&line, &len, file)) != -1) {
        printf("%s", line);
    }
    free(line);
}

void send_mail(char* address, char* topic, char* content) {
    printf("Sending mail...\nTitle: %s\nAddress: %s \nContent: %s\n\n", topic, address, content);

    char *command = malloc(sizeof (address) + sizeof (topic) + sizeof (content) + sizeof (char) * 32);

    sprintf(command, "echo %s | mail %s -s %s", content, address, topic);
    printf("%s\n", command);

    FILE* file = popen(command, "r");
    if (file == NULL){
        puts("Error popen().");
        exit(1);
    }
    free(command);
}

int main(int argc, char **argv) {
    if (argc == 2){
        get_mails(argv[1]);
    } else if(argc==4){
        send_mail(argv[1], argv[2], argv[3]);
    }else{
        printf("ERROR wrong number of arguments!");
        return 1;
    }
    return 0;
}