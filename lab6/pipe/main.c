#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2]; // [0] - чтение, [1] - запись

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    char buffer[128];
    time_t parent_time, child_time;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) { // Родительский процесс
        close(pipefd[0]);
        parent_time = time(NULL);
        snprintf(buffer, sizeof(buffer), "Parent PID: %d, Time: %s", getpid(), ctime(&parent_time));
        write(pipefd[1], buffer, strlen(buffer) + 1);
        close(pipefd[1]);
        wait(NULL);
    }
    else { // Дочерний процесс
        sleep(5);
        close(pipefd[1]);
        read(pipefd[0], buffer, sizeof(buffer));
        close(pipefd[0]);
        child_time = time(NULL);
        printf("Child Time: %s", ctime(&child_time));
        printf("Received: %s\n", buffer);
    }

    return 0;
}
