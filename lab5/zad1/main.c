#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

typedef struct Command {
    char **expression;
    int no_elements;
} Command;

typedef struct Dict {
    char *name;
    Command *commands;
    int no_commands;
} Dict;


// Realokacja pamięci i dodawanie kolejnego elementu do komendy
void add_new_element(Command *command, char* text_to_add){
    command->no_elements++;
    command->expression = realloc(command->expression, command->no_elements * sizeof (char *));
    command->expression[command->no_elements - 1] = malloc(sizeof (text_to_add));
    strcpy(command->expression[command->no_elements - 1], text_to_add);
}

// Przygotowanie komendy, na przykład: cat /etc/passwd -> ["cat", "/etc/passwd"]
Command make_command(char *tmp) {
    Command new_command;
    new_command.expression = malloc(sizeof(char**));
    new_command.no_elements = 0;

    char *tok = strtok(tmp, " \r\n\t");
    while(tok != NULL) {
        add_new_element(&new_command, tok);
        tok = strtok(NULL, " \r\n\t");
    }

    return new_command;
}

// Dodanie składnika do Dict
void add_to_dict(char *line, Dict *comp) {
    comp->no_commands = 1;
    for(int i = 0; i < strlen(line); i++) {
        if(line[i] == '|') {
            (comp->no_commands)++;
        }
    }

    char *name = strtok(line, "=");
    comp->name = malloc(sizeof name);
    sprintf(comp->name, "%.*s", (int)(strlen(name) - 1), name);

    comp->commands = malloc(sizeof (Command) * comp->no_commands);
    char **lines = malloc(sizeof (char*) * comp->no_commands);
    int id = 0;

    char *exp = strtok(NULL, "|");
    while(exp != NULL) {
        lines[id]= malloc(sizeof (exp));
        sprintf(lines[id++], "%s", exp + 1);
        exp = strtok(NULL, "|");
    }
    for(int i = 0; i < comp->no_commands; i++) {
        comp->commands[i] = make_command(lines[i]);
    }
    free(lines);
}

void execute_commands(Command **to_run, int cnt){
    int **fd = malloc(sizeof (int *) * cnt);
    for(int i = 0; i < cnt ; i++) {
        fd[i] = malloc(sizeof(int)*2);
        if (pipe(fd[i]) < 0) {
            exit(1);
        }
    }

    for(int i = 0; i < cnt; i++) {
        if (fork() == 0) {
            if(i > 0) {
                dup2(fd[i - 1][0], STDIN_FILENO);
            }
            if(i < cnt - 1) {
                dup2(fd[i][1], STDOUT_FILENO);
            }
            for(int j = 0; j < cnt; j++) {
                close(fd[j][1]);
                close(fd[j][0]);
            }
            execvp(to_run[i]->expression[0], to_run[i]->expression);
        }
    }

    for(int j = 0; j < cnt; j++) {
        close(fd[j][1]);
        close(fd[j][0]);
    }

    for (int i = 0; i < cnt; i++)
        wait(NULL);

    free(fd);
}

void add_to_exec(char *line, Dict *components, int no_components) {
    Command **to_run = malloc(sizeof (Command *));
    int id = -1;

    // strtok rozdziela tekst na poszczególne wyrazy, np. command1 | command2 | command3 => command1, command2, command3
    char *exp = strtok(line, "| \n\t\r");

    while(exp != NULL){
        // Sprawdzamy czy mamy w naszym zbiorze zdefiniowany dany składnik
        for(int i = 0; i < no_components; i++) {
            if(strcmp(exp, components[i].name)==0){
                // Jeśli dany składnik jest zdefiniowany to dodajemy komendy do 'to_run'
                for(int j = 0; j < components[i].no_commands; j++) {
                    for(int k = 0; k <  components[i].commands[j].no_elements; k++) {
                        to_run = realloc(to_run, (++id + 1)*sizeof(Command*));
                        to_run[id] = &components[i].commands[j];
                    }
                }
            }
        }
        exp = strtok(NULL, "| \n\t\r");
    }
    execute_commands(to_run, id + 1);
    free(to_run);
}

// Wczytuje zawartość pliku linia po linii i w zależnośći od tekstu definiuje składnik lub wykonuje potok
void read_file(char *file_name) {
    Dict *components = NULL;
    int no_components = 0;

    FILE *f = fopen(file_name, "r");
    if(f == NULL) {
        printf("Bład podczas otwarcia pliku %s!", file_name);
        exit(1);
    }

    char *line;
    size_t len = 0;
    while((getline(&line, &len, f)) != -1) {
        if(strchr(line, '=')) {
            printf("> Definicja składnika : %s", line);
            components = realloc(components, ++no_components * sizeof (Dict));
            add_to_dict(line, &components[no_components - 1]);
        } else {
            printf("> Wykonanie potoku : %s", line);
            add_to_exec(line, components, no_components);
        }
    }
    fclose(f);
    free(line);
    free(components);
}

int main(int argc, char **argv) {
    if(argc != 2) {
        puts("Błędna liczba argumentów!");
        return 1;
    }
    read_file(argv[1]);
    return 0;
}