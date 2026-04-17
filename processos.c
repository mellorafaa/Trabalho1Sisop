#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define TOTAL 1000000000
#define SEM_NAME "/meusem_sisop"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <N> <modo>\n", argv[0]);
        fprintf(stderr, "Modo deve ser 1 = sem semáforo (P1), 2 = com semáforo (P2)\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int modo = atoi(argv[2]);

    if (N <= 0) {
        fprintf(stderr, "Erro o N deve ser maior que 0\n");
        return 1;
    }
    if (modo != 1 && modo != 2) {
        fprintf(stderr, "Erro o modo deve ser 1 ou 2\n");
        return 1;
    }

    int shmid = shmget(IPC_PRIVATE, sizeof(long), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erro o shmget falhou");
        return 1;
    }

    long *contador = (long*) shmat(shmid, NULL, 0);
    if (contador == (long*) -1) {
        perror("Erro o shmat falhou");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }
    *contador = 0;

    sem_t *sem = NULL;
    if (modo == 2) {
        sem_unlink(SEM_NAME);
        sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
        if (sem == SEM_FAILED) {
            perror("Erro o sem_open falhou");
            shmdt(contador);
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }
    }

    long por_processo = TOTAL / N;

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    pid_t *pids = (pid_t*) malloc(N * sizeof(pid_t));
    if (pids == NULL) {
        fprintf(stderr, "Erro o malloc falhou ao alocar memória para PIDs\n");
        if (modo == 2) {
            sem_close(sem);
            sem_unlink(SEM_NAME);
        }
        shmdt(contador);
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            fprintf(stderr, "Erro o fork falhou no processo %d\n", i);
            if (modo == 2) {
                sem_close(sem);
                sem_unlink(SEM_NAME);
            }
            shmdt(contador);
            shmctl(shmid, IPC_RMID, NULL);
            free(pids);
            return 1;
        }

        if (pid == 0) {
            for (long j = 0; j < por_processo; j++) {
                if (modo == 2) {
                    if (sem_wait(sem) == -1) {
                        perror("sem_wait");
                    }
                }
                (*contador)++;
                if (modo == 2) {
                    if (sem_post(sem) == -1) {
                        perror("sem_post");
                    }
                }
            }
            if (modo == 2) {
                if (sem_close(sem) == -1) {
                    perror("sem_close");
                }
            }
            if (shmdt(contador) == -1) {
                perror("shmdt (filho)");
            }
            exit(0);
        }
        pids[i] = pid;
    }

    for (int i = 0; i < N; i++) {
        int status;
        pid_t waited_pid = waitpid(pids[i], &status, 0);
        if (waited_pid == -1) {
            perror("waitpid");
        } else if (!WIFEXITED(status)) {
            fprintf(stderr, "O processo %d não terminou normalmente\n", i);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &fim) == -1) {
        perror("clock_gettime (fim)");
    }
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("Valor final: %ld\n", *contador);
    printf("Tempo de execução: %.4f segundos\n", tempo);

    if (modo == 2) {
        if (sem_close(sem) == -1) {
            perror("sem_close (pai)");
        }
        if (sem_unlink(SEM_NAME) == -1) {
            perror("sem_unlink");
        }
    }

    if (shmdt(contador) == -1) {
        perror("shmdt (pai)");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    free(pids);
    return 0;
}