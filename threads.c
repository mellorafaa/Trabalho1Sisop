#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define TOTAL 1000000000

long contador = 0;
int N;
long por_thread;

pthread_mutex_t lock;

void* tarefa_sem_mutex(void* arg) {
    for (long i = 0; i < por_thread; i++) {
        contador++;
    }
    return NULL;
}

void* tarefa_com_mutex(void* arg) {
    for (long i = 0; i < por_thread; i++) {
        pthread_mutex_lock(&lock);
        contador++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <N> <modo>\n", argv[0]);
        fprintf(stderr, "Modo deve ser 1 = sem mutex (T1), 2 = com mutex (T2)\n");
        return 1;
    }

    N = atoi(argv[1]);
    int modo = atoi(argv[2]);

    if (N <= 0) {
        fprintf(stderr, "Erro o N deve ser maior que 0\n");
        return 1;
    }
    if (modo != 1 && modo != 2) {
        fprintf(stderr, "Erro o modo deve ser 1 ou 2\n");
        return 1;
    }

    pthread_t *threads = (pthread_t*) malloc(N * sizeof(pthread_t));
    if (threads == NULL) {
        fprintf(stderr, "Erro ao alocar memória para threads\n");
        return 1;
    }

    por_thread = TOTAL / N;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Erro ao inicializar mutex\n");
        free(threads);
        return 1;
    }

    struct timespec inicio, fim;
    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) {
        perror("clock_gettime (inicio)");
        pthread_mutex_destroy(&lock);
        free(threads);
        return 1;
    }

    for (int i = 0; i < N; i++) {
        int ret;
        if (modo == 1)
            ret = pthread_create(&threads[i], NULL, tarefa_sem_mutex, NULL);
        else
            ret = pthread_create(&threads[i], NULL, tarefa_com_mutex, NULL);

        if (ret != 0) {
            fprintf(stderr, "Erro ao criar thread %d\n", i);
            pthread_mutex_destroy(&lock);
            free(threads);
            return 1;
        }
    }

    for (int i = 0; i < N; i++) {
        int ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "Erro ao aguardar thread %d\n", i);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &fim) != 0) {
        perror("clock_gettime (fim)");
        pthread_mutex_destroy(&lock);
        free(threads);
        return 1;
    }

    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("Valor final: %ld\n", contador);
    printf("Tempo de execução: %.4f segundos\n", tempo);

    pthread_mutex_destroy(&lock);
    free(threads);
    return 0;
}