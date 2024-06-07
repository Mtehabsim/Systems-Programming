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

#define PORT 12346
#define ARRAY_SIZE 10
#define DIRECTORY_NAME "from_clients"
#define HASH_LENGTH 64
#define BUFFER_SIZE 1024

void *hash_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    free(socket_desc);
    char filename[256] = {0};
    
    if (recv(sock, filename, sizeof(filename) - 1, 0) <= 0) {
        perror("Receive filename failed");
        close(sock);
        return NULL;
    }
    printf("File name is %s\n", filename);
    
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", DIRECTORY_NAME, filename);
    mkdir(DIRECTORY_NAME, 0777);

    int file_fd = open(file_path, O_CREAT | O_WRONLY, 0644);
    if (file_fd < 0) {
        perror("File open failed");
        close(sock);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        if (write(file_fd, buffer, bytes_received) != bytes_received) {
            perror("File write failed");
            close(file_fd);
            close(sock);
            return NULL;
        }
    }

    if (bytes_received < 0) {
        perror("Receive file content failed");
    }

    close(file_fd);

    char *hash = hash_file(file_path);
    if (hash) {
        send(sock, hash, HASH_LENGTH, 0);
        free(hash);
    }

    close(sock);
    return NULL;
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

        if (pthread_create(&thread_id, NULL, hash_handler, (void *)new_sock) != 0) {
            perror("could not create thread");
            free(new_sock);
        }

        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}
