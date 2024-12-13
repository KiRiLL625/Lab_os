#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define FTOK_PATH "."
#define MAX_LEN 256

typedef struct {
    pid_t pid;
    char time_str[MAX_LEN];
    int data_ready;
} shared_data;

int shmid;

//Ctrl+C
void handle_sigint(int sig) {
    if(shmctl(shmid, IPC_RMID, NULL)){
        perror("shmctl");
    }
    else{
        printf("SIGINT received, shmem deleted\n");
    }
    exit(0);
}

int main() {
    key_t shm_key;
    shared_data *shm_ptr;

    signal(SIGINT, handle_sigint);

    //Проверка существования разделяемой памяти
    shm_key = ftok(FTOK_PATH, 'A');
    if (shm_key == -1) {
        perror("ftok");
        exit(1);
    }

    shmid = shmget(shm_key, MAX_LEN, IPC_CREAT | IPC_EXCL | 0666);
    if(shmid == -1){
        perror("shmget");
        printf("Shared memory already exists\n");
        exit(1);
    }

    shm_ptr = (shared_data*) shmat(shmid, NULL, 0);
    if(shm_ptr == (void*) -1){
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
    }

    while (1) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_str[MAX_LEN];
        strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);

        shm_ptr->pid = getpid();
        strncpy(shm_ptr->time_str, time_str, MAX_LEN);
        shm_ptr->data_ready = 1;

        sleep(1);
    }

    return 0;
}
