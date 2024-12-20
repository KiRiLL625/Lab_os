#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
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

shared_data* shm_ptr;

void handle_sigint(int arg){
    shmdt(shm_ptr);
    printf("Shmem detached\n");
    exit(0);
}

int main() {
    int shmid;
    int semid;
    key_t key;
    signal(SIGINT, handle_sigint);

    //Открытие разделяемой памяти
    key = ftok(FTOK_PATH, 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    semid = semget(key, 1, 0666);
    if(semid == -1){
        perror("semget");
        printf("Sender is not running\n");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl_setval");
        semctl(semid, 0, IPC_RMID);
        exit(1);
    }

    //Установка размера разделяемой памяти
    shmid = shmget(key, MAX_LEN, 0666);
    if(shmid == -1){
        perror("shmget");
        printf("Cannot find shared memory\n");
        exit(1);
    }

    shm_ptr = (shared_data*) shmat(shmid, NULL, 0);
    if(shm_ptr == (void*) -1){
        perror("shmat");
        exit(1);
    }

    struct sembuf sem = {0, -1, 0};

    while (1) {
        if(semop(semid, &sem, 1) == -1){
            perror("semop");
            exit(1);
        }
        //if (sem.sem_op == 1) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char time_str[MAX_LEN];
            strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);

            printf("Receiver PID: %d, Time: %s, Sender PID: %d, Sender Time: %s\n",
                   getpid(), time_str, shm_ptr->pid, shm_ptr->time_str);
            //shm_ptr->data_ready = 0;
            //sem.sem_op = -1;
        //}

        sleep(1);
    }

    return 0;
}
