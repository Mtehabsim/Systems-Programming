#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define MAIN_PORT 12345
#define BUFFER_SIZE 1024
#define ARRAY_SIZE 10

void connect_to_server(int *sockfd, struct sockaddr_in *serv_addr);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <choice> <parameters...>\n", argv[0]);
        exit(1);
    }

    int choice = atoi(argv[1]);
    connect_to_server(&sockfd, &serv_addr);

    // Send the choice to the server
    write(sockfd, &choice, sizeof(choice));

    if (choice == 1) {
        if (argc != 13) {  // Choice + 10 integers + 1 target
            fprintf(stderr, "Usage for choice 1: %s 1 <10 integers> <target>\n", argv[0]);
            exit(1);
        }
        int array[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = atoi(argv[i + 2]);
        }
        int target = atoi(argv[12]);

        // Send array and target to server
        write(sockfd, array, sizeof(array));
        write(sockfd, &target, sizeof(target));

        // Receive sorted array and index from the server
        read(sockfd, array, sizeof(array));
        int index;
        read(sockfd, &index, sizeof(index));

        printf("Sorted Array:\n");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", array[i]);
        }
        printf("\nTarget found at index: %d\n", index);
    } else if (choice == 2) {
        if (argc != 3) {
            fprintf(stderr, "Usage for choice 2: %s 2 <filename>\n", argv[0]);
            exit(1);
        }
        char *filename = argv[2];
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("Failed to open file");
            exit(1);
        }

        // Read file content
        char buffer[BUFFER_SIZE];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
        fclose(file);

        // Send file content to server
        write(sockfd, buffer, bytes_read);

        // Receive hash value from the server
        char hash_value[BUFFER_SIZE];
        read(sockfd, hash_value, sizeof(hash_value));
        printf("Hash Value: %s\n", hash_value);
    }

    close(sockfd);
    return 0;
}

void connect_to_server(int *sockfd, struct sockaddr_in *serv_addr) {
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset(serv_addr, 0, sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(MAIN_PORT);
    serv_addr->sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(*sockfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
}
