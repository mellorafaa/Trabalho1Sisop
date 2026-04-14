#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

#define TOTAL 1000000000
#define SEM_NAME "/meusem_sisop"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <N> <modo>\n", argv[0]);
        fprintf(stderr, "modo: 1 = sem semáforo (P1), 2 = com semáforo (P2)\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int modo = atoi(argv[2]);

    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser maior que 0\n");
        return 1;
    }
    if (modo != 1 && modo != 2) {
        fprintf(stderr, "Erro: modo deve ser 1 ou 2\n");
        return 1;
    }

    int shmid = shmget(IPC_PRIVATE, sizeof(long), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erro: shmget falhou");
        return 1;
    }

    long *contador = (long*) shmat(shmid, NULL, 0);
    if (contador == (long*) -1) {
        perror("Erro: shmat falhou");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }
    *contador = 0;

    sem_t *sem = NULL;
    if (modo == 2) {
        /* Remove semáforo de execução anterior para evitar estado inconsistente */
        sem_unlink(SEM_NAME);
        sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
        if (sem == SEM_FAILED) {
            perror("Erro: sem_open falhou");
            shmdt(contador);
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }
    }

    long por_processo = TOTAL / N;

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            fprintf(stderr, "Erro: fork falhou no processo %d\n", i);
            if (modo == 2) {
                sem_close(sem);
                sem_unlink(SEM_NAME);
            }
            shmdt(contador);
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }

        if (pid == 0) {
            for (long j = 0; j < por_processo; j++) {
                if (modo == 2) sem_wait(sem);
                (*contador)++;
                if (modo == 2) sem_post(sem);
            }
            if (modo == 2) sem_close(sem);
            shmdt(contador);
            exit(0);
        }
    }

    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    printf("Valor final: %ld\n", *contador);

    if (modo == 2) {
        sem_close(sem);
        sem_unlink(SEM_NAME);
    }

    shmdt(contador);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}