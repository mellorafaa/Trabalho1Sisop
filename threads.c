#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
        fprintf(stderr, "modo: 1 = sem mutex (T1), 2 = com mutex (T2)\n");
        return 1;
    }

    N = atoi(argv[1]);
    int modo = atoi(argv[2]);

    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser maior que 0\n");
        return 1;
    }
    if (modo != 1 && modo != 2) {
        fprintf(stderr, "Erro: modo deve ser 1 ou 2\n");
        return 1;
    }

    pthread_t threads[N];
    por_thread = TOTAL / N;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Erro: falha ao inicializar mutex\n");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        int ret;
        if (modo == 1)
            ret = pthread_create(&threads[i], NULL, tarefa_sem_mutex, NULL);
        else
            ret = pthread_create(&threads[i], NULL, tarefa_com_mutex, NULL);

        if (ret != 0) {
            fprintf(stderr, "Erro: falha ao criar thread %d\n", i);
            pthread_mutex_destroy(&lock);
            return 1;
        }
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Valor final: %ld\n", contador);

    pthread_mutex_destroy(&lock);
    return 0;
}