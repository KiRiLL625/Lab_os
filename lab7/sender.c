#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define SHM_NAME "/shmemory"
#define MAX_LEN 256

typedef struct {
    pid_t pid;
    char time_str[MAX_LEN];
    int data_ready;
} shared_data;

//Ctrl+C
void handle_sigint(int sig) {
    shm_unlink(SHM_NAME);
    perror("SIGINT received, exiting");
    exit(0);
}

int main() {
    int shm_fd;
    shared_data *shm_ptr;

    signal(SIGINT, handle_sigint);

    //Проверка наличия разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Shared memory already exists or cannot be created");
        exit(1);
    }

    //Установка размера разделяемой памяти
    ftruncate(shm_fd, sizeof(shared_data));

    //Отображение разделяемой памяти в адресное пространство
    shm_ptr = mmap(0, sizeof(shared_data), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
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
