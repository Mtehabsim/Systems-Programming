#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 12345
#define HASH_LENGTH 64
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    const char *filename = argv[1];
    int sockfd, file_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, filename, strlen(filename) + 1, 0) < 0) { // Include null-terminator
        perror("Send filename failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("File open failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) < 0) {
            perror("Send file content failed");
            close(file_fd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0) {
        perror("File read failed");
        close(file_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    } else {
        printf("File sent successfully\n");
    }

    shutdown(sockfd, SHUT_WR);

    char hash[HASH_LENGTH + 1] = {0};
    if (recv(sockfd, hash, HASH_LENGTH, 0) < 0) {
        perror("Receive hash failed");
        close(file_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Received hash: %s\n", hash);

    close(file_fd);
    close(sockfd);
    return 0;
}
