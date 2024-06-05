#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include "common.h"
#define PORT 12345
#define ARRAY_SIZE 10

void *client_handler(void *arg);
void write_to_pipe(int *array, int size, int pipe_fd) {
    write(pipe_fd, array, size * sizeof(int));
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(sockfd);
        exit(1);
    }

    listen(sockfd, 5);
    printf("Server is listening on port %d\n", PORT);

    while (1) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue;
        }

        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));
        *new_sock = newsockfd;

        if (pthread_create(&thread_id, NULL, client_handler, (void *)new_sock) != 0) {
            perror("could not create thread");
            free(new_sock);
        }

        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}

void *client_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    free(socket_desc);

    int array[ARRAY_SIZE];
    int target;
    int read_size;

    // Read the entire array
    read_size = read(sock, array, sizeof(int) * ARRAY_SIZE);
    if (read_size < sizeof(int) * ARRAY_SIZE) {
        perror("Read array failed");
        close(sock);
        return NULL;
    }

    // Read the target value
    read_size = read(sock, &target, sizeof(int));
    if (read_size < sizeof(int)) {
        perror("Read target failed");
        close(sock);
        return NULL;
    }

    int pipes[4][2];
    for (int i = 0; i < 4; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed");
            close(sock);
            return NULL;
        }
    }

    pid_t pid_sort = fork();
    if (pid_sort == 0) {
        clock_t start, end;
        double cpu_time_used;
        start = clock();

        if (fork() == 0) {
            quicksort(array, 0, ARRAY_SIZE - 1);
            close(pipes[0][0]);
            write_to_pipe(array, ARRAY_SIZE, pipes[0][1]);
            printf("Quicksort finished\n");
            exit(0);
        }
        if (fork() == 0) {
            insertionSort(array, ARRAY_SIZE);
            close(pipes[1][0]);
            write_to_pipe(array, ARRAY_SIZE, pipes[1][1]);
            printf("Insertion sort finished\n");
            exit(0);
        }
        if (fork() == 0) {
            Merge_Sort(array, 0, ARRAY_SIZE - 1);
            close(pipes[2][0]);
            write_to_pipe(array, ARRAY_SIZE, pipes[2][1]);
            printf("Merge sort finished\n");
            exit(0);
        }
        if (fork() == 0) {
            bubblesort(array);
            close(pipes[3][0]);
            write_to_pipe(array, ARRAY_SIZE, pipes[3][1]);
            printf("Bubble sort finished\n");
            exit(0);
        }

        while (wait(NULL) > 0);
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Sorting processes took %f seconds to execute\n", cpu_time_used);
        exit(0);
    } else {
        waitpid(pid_sort, NULL, 0);

        clock_t start, end;
        double cpu_time_used;
        start = clock();

        if (fork() == 0) {
            read(pipes[0][0], array, ARRAY_SIZE * sizeof(int));
            int index = binary_search(array, 0, ARRAY_SIZE - 1, target);
            int count = count_frequency(array, ARRAY_SIZE, index, target);
            write(sock, array, sizeof(int) * ARRAY_SIZE);
            write(sock, &count, sizeof(int));
            exit(0);
        }
        if (fork() == 0) {
            read(pipes[1][0], array, ARRAY_SIZE * sizeof(int));
            int index = fibonacci_search(array, ARRAY_SIZE, target);
            int count = count_frequency(array, ARRAY_SIZE, index, target);
            write(sock, array, sizeof(int) * ARRAY_SIZE);
            write(sock, &count, sizeof(int));
            exit(0);
        }
        if (fork() == 0) {
            read(pipes[2][0], array, ARRAY_SIZE * sizeof(int));
            int index = jump_search(array, ARRAY_SIZE, target);
            int count = count_frequency(array, ARRAY_SIZE, index, target);
            write(sock, array, sizeof(int) * ARRAY_SIZE);
            write(sock, &count, sizeof(int));
            exit(0);
        }
        if (fork() == 0) {
            read(pipes[3][0], array, ARRAY_SIZE * sizeof(int));
            int index = ternary_search(array, 0, ARRAY_SIZE, target);
            int count = count_frequency(array, ARRAY_SIZE, index, target);
            write(sock, array, sizeof(int) * ARRAY_SIZE);
            write(sock, &count, sizeof(int));
            exit(0);
        }

        while (wait(NULL) > 0);
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Searching processes took %f seconds to execute\n", cpu_time_used);
    }

    close(sock);
    return NULL;
}
