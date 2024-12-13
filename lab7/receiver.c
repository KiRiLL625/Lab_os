#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#define SHM_NAME "/shmemory"
#define MAX_LEN 256

typedef struct {
    pid_t pid;
    char time_str[MAX_LEN];
    int data_ready;
} shared_data;

int main() {
    int shm_fd;
    shared_data *shm_ptr;

    //Открытие разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    //Установка размера разделяемой памяти
    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    //Отображение разделяемой памяти в адресное пространство
    shm_ptr = mmap(0, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
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
