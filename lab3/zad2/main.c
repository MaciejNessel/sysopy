#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <math.h>
#include <string.h>
#include <stdio.h>


double f(double x){
    if(x<0 || x>1)
        return 0;
    return 4 / (x * x + 1);
}


char* get_filename(int n) {
    char number[10];
    sprintf(number, "%d", n);

    char* result = malloc(10*sizeof(char) + sizeof(number));
    result[0] = '\0';

    strcat(result, "./temp/w");
    strcat(result, number);
    strcat(result, ".txt");

    return result;
}


// Stworzenie nowego procesu potomnego i wykonanie obliczeń dla danego procesu
void new_process(int i, double h, int curr_process, int per_process){
    pid_t pid = fork();
    if(pid == 0){
        double area = 0.0;

        FILE *file = NULL;
        file = fopen(get_filename(curr_process), "w");
        if(file == NULL){
            printf("Error opening file on process function.\n");
            return;
        }

        for(int j = 0; j < per_process; j++){
            area += f(i * h) * h;
            i++;
        }

        fprintf(file, "%f", area);
        fclose(file);
        exit(0);
    }
    else if(pid == -1){
        printf("Error in fork.\n");
        exit(1);
    }
}


// Wyświetlenie wyników końcowych
double get_result(int i){
    FILE *file = NULL;
    file = fopen(get_filename(i), "r");
    if(file == NULL){
        printf("Error opening file from temp.\n");
        exit(0);
    }
    double area = 0.0;
    fscanf(file, "%lf", &area);
    fclose(file);

    return area;
}


int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Wrong number of arguments.\n");
        return 0;
    }

    // h - długość przedziału, n - liczba procesów
    double h = atof(argv[1]);
    int n = atoi(argv[2]);

    if(n < 1){
        printf("The number of processes is to small.\n");
        return 0;
    }
    if(h == 0){
        printf("The compartment length is to small.\n");
        return 0;
    }

    // no_compartments - liczba przedziałów, per_process - liczba obliczeń na jeden proces
    int no_compartments = ceil(1 / h) + 1;
    int per_process = ceil((double) no_compartments / (double) n);

    int i = 0;
    int curr_process = 0;
    while (i < no_compartments){
        while(curr_process < n){
            new_process(i, h, curr_process, per_process);
            i+=per_process;
            curr_process++;
        }
    }

    // Oczekiwanie na procesy potomne
    for (int i = 0; i < n; i++){
        if(wait(0) < 0){
            printf("Error in wait.\n");
            return 1;
        }
    }

    // Obliczanie wyników
    double result = 0.0;
    for (int i = 0; i < n; i++){
        result += get_result(i);
    }

    printf("Result: %lf\n", result);

    return 0;
}
