#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include "common.h"

#define PORT 12345
#define ARRAY_SIZE 10
#define DIRECTORY_NAME "from_clients"
#define HASH_LENGTH 64

void *client_handler(void *arg);

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
    char filename[256];
    int target;

    if (read(sock, array, sizeof(int) * ARRAY_SIZE) != sizeof(int) * ARRAY_SIZE) {
        perror("Read array failed");
        close(sock);
        return NULL;
    }

    if (read(sock, &target, sizeof(int)) != sizeof(int)) {
        perror("Read target failed");
        close(sock);
        return NULL;
    }

    if (read(sock, filename, sizeof(filename)) <= 0) {
        perror("Read filename failed");
        close(sock);
        return NULL;
    }

    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", DIRECTORY_NAME, filename);

    mkdir(DIRECTORY_NAME, 0777);

    int file_fd = open(file_path, O_CREAT | O_WRONLY, 0644);
    if (file_fd < 0) {
        perror("File open failed");
        close(sock);
        return NULL;
    }

   int pipe_hash[2];
    if (pipe(pipe_hash) < 0) {
        perror("Error creating pipe for hashing");
        close(file_fd);
        close(sock);
        return NULL;
    }

    pid_t pid_hash = fork();
    if (pid_hash == 0) { 
        close(pipe_hash[0]);

        char *hash = hash_file(file_path);
        if (hash) {
            write(pipe_hash[1], hash, HASH_LENGTH);
            free(hash);
        }
        close(pipe_hash[1]);
        exit(0);
    } else if (pid_hash > 0) {
        close(pipe_hash[1]);

        int pipe_sort[2];
        if (pipe(pipe_sort) < 0) {
            perror("Error creating pipe for sorting");
            close(pipe_hash[0]);
            close(file_fd);
            close(sock);
            return NULL;
        }

        pid_t pid_sort = fork();
        if (pid_sort == 0) {
            close(pipe_sort[0]);
            quicksort(array, 0, ARRAY_SIZE - 1);
            write(pipe_sort[1], array, sizeof(int) * ARRAY_SIZE);
            close(pipe_sort[1]);
            exit(0);
        } else if (pid_sort > 0) {
            close(pipe_sort[1]); 

            int index;
            read(pipe_sort[0], array, sizeof(int) * ARRAY_SIZE);

            pid_t pid_search = fork();
            if (pid_search == 0) {
                int found_index = binary_search(array, 0, ARRAY_SIZE - 1, target);
                exit(found_index);
            } else if (pid_search > 0) {
                int search_status;
                waitpid(pid_search, &search_status, 0);
                index = WEXITSTATUS(search_status);

                char hash[HASH_LENGTH];
                read(pipe_hash[0], hash, HASH_LENGTH);
                write(sock, array, sizeof(int) * ARRAY_SIZE);
                write(sock, &index, sizeof(int));
                write(sock, hash, HASH_LENGTH);
            }
        }
        close(pipe_sort[0]);
        close(pipe_hash[0]);
    }

    close(file_fd);
    close(sock);
    return NULL;
}