#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/sem.h>

#define SHM_NAME "/shmemory"
#define MAX_LEN 256
#define SEM_KEY 52

typedef struct {
    pid_t pid;
    char time_str[MAX_LEN];
} shared_data;

int main() {
    int shm_fd;
    shared_data *shm_ptr;
    int sem_id;
    struct sembuf sem_op;

    // Открытие разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Установка размера разделяемой памяти
    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Отображение разделяемой памяти в адресное пространство
    shm_ptr = mmap(0, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Получение семафора
    sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        perror("semget");
        exit(1);
    }

    while (1) {
        // Ожидание sender
        sem_op.sem_num = 0;
        sem_op.sem_op = -1; //Уменьшение семафора
        sem_op.sem_flg = 0;
        if (semop(sem_id, &sem_op, 1) == -1) {
            perror("semop");
            exit(1);
        }

        if (sem_op.sem_op == -1) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char time_str[MAX_LEN];
            strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);

            printf("Receiver PID: %d, Time: %s, Sender PID: %d, Sender Time: %s\n",
                   getpid(), time_str, shm_ptr->pid, shm_ptr->time_str);
        }

        sleep(1);
    }

    return 0;
}
