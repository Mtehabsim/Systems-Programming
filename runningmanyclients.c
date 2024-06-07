#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void execute_program(const char *program, const char *arg1, const char *arg2) {
    execl(program, program, arg1, arg2, (char *)NULL);
    perror("execl failed");
    exit(EXIT_FAILURE);
}
int main() {
    pid_t pid;
    for (int i = 0; i < 3; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            execute_program("./client", "2", "Name.txt");
        }
    }
    for (i = 0; i < 3; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            execute_program("./client", "1", NULL);
        }
    }
    for (i = 0; i < 6; i++) {
        wait(NULL);
    }
    return 0;
}
