#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define FTOK_PATH "."
#define MAX_LEN 256

typedef struct {
    pid_t pid;
    char time_str[MAX_LEN];
    //int data_ready;
} shared_data;

int shmid;
int semid;

shared_data *shm_ptr;

//Ctrl+C
void handle_sigint(int sig) {
    shmdt(shm_ptr);
    if(shmctl(shmid, IPC_RMID, NULL)){
        perror("shmctl");
    }
    else{
        printf("SIGINT received, shmem deleted\n");
    }

    if(semctl(semid, 0, IPC_RMID)){
        perror("semctl");
    }
    else{
        printf("Semaphore deleted\n");
    }
    exit(0);
}

int main() {
    key_t key;

    signal(SIGINT, handle_sigint);

    //Проверка существования разделяемой памяти
    key = ftok(FTOK_PATH, 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if(semid == -1){
        perror("semget");
        printf("Sender already running\n");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl_setval");
        semctl(semid, 0, IPC_RMID);
        exit(1);
    }

    shmid = shmget(key, MAX_LEN, IPC_CREAT | IPC_EXCL | 0666);
    if(shmid == -1){
        perror("shmget");
        printf("Shared memory already exists\n");
        exit(1);
    }

    shm_ptr = (shared_data*) shmat(shmid, NULL, 0);
    if(shm_ptr == (void*) -1){
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
    }

    struct sembuf sem = {0, 0, 0};

    while (1) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_str[MAX_LEN];
        strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);

        shm_ptr->pid = getpid();
        strncpy(shm_ptr->time_str, time_str, MAX_LEN);
        sem.sem_op = 1;
        if (semop(semid, &sem, 1) == -1) {
            perror("semop");
            exit(1);
        }

        sleep(1);
    }

    return 0;
}
