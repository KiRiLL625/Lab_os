#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#define FTOK_PATH "."
#define MAX_LEN 256

typedef struct {
    pid_t pid;
    char time_str[MAX_LEN];
    int data_ready;
} shared_data;

shared_data* shm_ptr;

void hangle_sigint(int arg){
    shmdt(shm_ptr);
    printf("Shmem detached\n");
}

int main() {
    int shmid;
    key_t shm_key;

    //Открытие разделяемой памяти
    shm_key = ftok(FTOK_PATH, 'A');
    if (shm_key == -1) {
        perror("ftok");
        exit(1);
    }

    //Установка размера разделяемой памяти
    shmid = shmget(shm_key, MAX_LEN, 0666);
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

    while (1) {
        if (shm_ptr->data_ready) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char time_str[MAX_LEN];
            strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);

            printf("Receiver PID: %d, Time: %s, Sender PID: %d, Sender Time: %s\n",
                   getpid(), time_str, shm_ptr->pid, shm_ptr->time_str);
            shm_ptr->data_ready = 0;
        }

        sleep(1);
    }

    return 0;
}
