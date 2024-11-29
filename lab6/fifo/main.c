#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FIFO_BUFFER "./fifo/my_fifo"

int main() {
    pid_t pid;
    char buffer[128];
    time_t parent_time, child_time;

    if (mkfifo(FIFO_BUFFER, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) { // Родительский процесс
        int fd;
        parent_time = time(NULL);
        snprintf(buffer, sizeof(buffer), "Parent PID: %d, Time: %s", getpid(), ctime(&parent_time));
        fd = open(FIFO_BUFFER, O_WRONLY);
        write(fd, buffer, strlen(buffer) + 1);
        close(fd);
        wait(NULL);
    }
    else { // Дочерний процесс
        sleep(5);
        int fd;
        fd = open(FIFO_BUFFER, O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        close(fd);
        child_time = time(NULL);
        printf("Child Time: %s", ctime(&child_time));
        printf("Received: %s\n", buffer);
        unlink(FIFO_BUFFER);
    }

    return 0;
}
