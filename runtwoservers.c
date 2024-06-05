#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void execute_program(const char *program) {
    execl(program, program, NULL);
    perror("execl failed");
    exit(EXIT_FAILURE);
}

int main() {
    pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid1 == 0) {
        printf("Sorting server (pid: %d)\n", getpid());
        execute_program("./sortserver");
    }
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid2 == 0) {
        printf("Hashing server (pid: %d)\n", getpid());
        execute_program("./hashserver");
    }
    wait(NULL);
    return 0;
}
