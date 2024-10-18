#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

void atexit_func(){
    printf("[ATEXIT] Atexit process, id: %d\n", getpid());
}

void handler(int signum){
    switch(signum){
        case SIGINT:
            printf("[SIGNAL] SIGINT signal received, pid: %d\n", getpid());
            break;
        case SIGTERM:
            printf("[SIGNAL] SIGTERM signal received, pid: %d\n", getpid());
            break;
        default:
            printf("[SIGNAL] Signal %d received, pid: %d\n", signum, getpid());
            break;
    }
}

int main(void) {
    if(signal(SIGINT, handler) == SIG_ERR){
        printf("Cannot catch SIGINT\n");
    }
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = SA_SIGINFO;

    if(sigaction(SIGTERM, &sa, NULL) == -1){
        printf("Cannot catch SIGTERM\n");
    }

    if(atexit(atexit_func) != 0){
        perror("Atexit failed\n");
    }

    pid_t pid = fork();

    if (pid < 0) {
        int err = errno;
        fprintf(stderr, "Fork failed: %s\n", strerror(err));
        return 1;
    } else if (pid == 0) {
        printf("[CHILD] This is the child process, my pid: %d, parent pid: %d\n", getpid(), getppid());
        sleep(10);
    } else {
        int status;
        wait(&status);
        printf("[PARENT] This is the parent process, pid: %d, child pid is %d, parent pid is %d\n", getpid(), pid, getppid());
        printf("[PARENT} Child process exited with status %d\n", WEXITSTATUS(status));
    }

    return 0;
}
