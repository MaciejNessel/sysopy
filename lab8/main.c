#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

int no_threads;
int height;
int width;
int** input_matrix;
int** result_matrix;


void count_time(struct timespec start, struct timespec end, double *time_value){
    int result = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000.0;
    *time_value = 1.0 * result/1000000.0;
}

void read_matrix(FILE* file){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fscanf(file, "%d", &input_matrix[i][j]);
        }
    }
}

void read_headers(FILE* file){
    char buffer[10];
    fscanf(file, "%s", buffer);
    fscanf(file, "%d %d", &width, &height);
    fscanf(file, "%d", (int*) buffer);
}

void read_input(char* filename){
    FILE* file = fopen(filename, "r");
    if(file == NULL){
        puts("No such file");
        exit(1);
    }
    read_headers(file);

    // Alokacja pami�ci dla macierzy wej�ciowej oraz wyj�ciowej
    input_matrix = calloc(height, sizeof(int*));
    result_matrix = calloc(height, sizeof(int*));
    for(int i = 0; i < height; i++){
        input_matrix[i] = calloc(width, sizeof(int));
        result_matrix[i] = calloc(width, sizeof(int));
    }

    // Wczytanie macierzy wej�ciowej
    read_matrix(file);

    fclose(file);
}

// Zapisuje macierz wyj�ciow� do pliku
void write_result_to_file(char* filename){
    FILE* file = fopen(filename, "w");
    if(file == NULL){
        puts("File opening failed.");
        exit(1);
    }

    fprintf(file, "%s\n", "P2");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "%d\n", 255);

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fprintf(file, "%d ", result_matrix[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}


// Wariant 1: Ka�dy w�tek wyznacza warto�ci pikseli obrazu wyj�ciowego tylko dla tych pikseli obrazu wej�ciowego,
// kt�re przyjmuj� warto�ci z okre�lonego zbioru/przedzia�u warto�ci.
// Zbi�r liczb dla w�tku mo�na przydzieli� w dowolny spos�b, ale taki, by ka�dy w�tek dosta� inne liczby i zadanie
// by�o podzielone r�wno na wszystkie w�tki.
void numbers_mode(int thread_id){
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if(input_matrix[i][j] % no_threads == thread_id){
                result_matrix[i][j] = 255 - input_matrix[i][j];
            }
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);

    double *time_val = malloc(sizeof (double ));
    count_time(start, end, time_val);

    pthread_exit(time_val);
}


// Wariant 2: Podzia� blokowy � k-ty w�tek oblicza warto�ci pikseli w pionowym pasku o wsp�rz�dnych
// x-owych w przedziale od (??1)?ceil(?/?) do ??ceil(?/?)?1, gdzie ? to szeroko�� obrazu wej�ciowego
// a ? to liczba stworzonych w�tk�w.
void block_mode(int k){
    struct timespec start, end;

    int left_range = (k-1)*ceil(width/no_threads);
    int right_range = k*ceil(width/no_threads) - 1;

    clock_gettime(CLOCK_REALTIME, &start);
    for(int i = 0; i < height; i++){
        for(int j = left_range; j <= right_range; j++){
            result_matrix[i][j] = 255 - input_matrix[i][j];
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);

    double *time_val = malloc(sizeof (double ));
    count_time(start, end, time_val);

    pthread_exit(time_val);
}


int main(int argc, char** argv){
    if(argc != 5) {
        puts("Wrong number of arguments.");
        exit(1);
    }

    // Program przyjmuje argumenty: 1. liczba w�tk�w, 2. tryb numbers/block, 3.plik wej�ciowy *.pgm 4.plik wynikowy
    no_threads = atoi(argv[1]);
    char mode[20];
    strcpy(mode, argv[2]);
    char input_filename[100];
    strcpy(input_filename, argv[3]);
    char output_filename[100];
    strcpy(output_filename, argv[4]);
    printf("* No Threads: [%d] \n* Mode: [%s]\n", no_threads, mode);
    read_input(input_filename);

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    // Alokacja pami�ci dla identyfikator�w w�tk�w
    pthread_t* threads_set = calloc(no_threads, sizeof(pthread_t));

    // Wyb�r trybu pracy programu (numbers / block) oraz utworzenie w�tk�w
    if(strcmp(mode, "numbers") == 0){
        for(int i = 0; i < no_threads; i++){
            pthread_create(&threads_set[i], NULL, (void*)numbers_mode, i);
        }
    }
    else if(strcmp(mode, "block") == 0){
        for(int i = 0; i < no_threads; i++){
            pthread_create(&threads_set[i], NULL, (void*)block_mode, i+1);
        }
    }
    else{
        puts("Wrong mode.");
        exit(1);
    }

    // Wy�wietla czas pracy poszczeg�lnych w�tk�w (kt�ry jest zwracany za pomoc� pthread_exit())
    for(int i = 0; i < no_threads; i++){
        double *result;
        pthread_join(threads_set[i], (void*)&result);
        printf("Thread %d - time: %lf\n", i, *result);
    }

    clock_gettime(CLOCK_REALTIME, &end);

    // Czas pracy g��wnego w�tku
    double *time_val = malloc(sizeof (double ));
    count_time(start, end, time_val);
    printf("Main thread - time: %lf\n", *time_val);

    // Zapisuje obliczone wyniki do pliku wyj�ciowego
    write_result_to_file(output_filename);

    // Zwalnianie pami�ci
    for(int i = 0; i < height; i++){
        free(input_matrix[i]);
        free(result_matrix[i]);
    }
    free(input_matrix);
    free(result_matrix);
    free(threads_set);
    return 0;
}