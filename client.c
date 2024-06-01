#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <math.h>
#include <time.h>

#define PORT 12345
#define SERVER_IP "127.0.0.1"
#define ARRAY_SIZE 10
#define HASH_LENGTH 64

void fill_array_with_random(int *array, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

int main() {
    int sockfd, file_fd;
    struct sockaddr_in servaddr;
    int array[ARRAY_SIZE];
    char filename[] = "Name.txt";
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    fill_array_with_random(array, ARRAY_SIZE);
    int target = array[0];
    if (send(sockfd, array, sizeof(array), 0) < 0) {
        perror("Send array failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, &target, sizeof(target), 0) < 0) {
        perror("Send target failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, filename, sizeof(filename), 0) < 0) {
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
        printf("Read %d bytes from file\n", bytes_read);
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

    close(file_fd);

    if (recv(sockfd, array, sizeof(array), 0) < 0) {
        perror("Receive sorted array failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int index;
    if (recv(sockfd, &index, sizeof(index), 0) < 0) {
        perror("Receive index failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char hash[HASH_LENGTH + 1];
    if (recv(sockfd, hash, HASH_LENGTH, 0) < 0) {
        perror("Receive hash failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    hash[HASH_LENGTH] = '\0';

    printf("Sorted array: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\nIndex of target %d: %d\n", target, index);
    printf("File hash: %s\n", hash);

    close(sockfd);
    return 0;
}
