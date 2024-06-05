#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PORT_SORTING 12345
#define PORT_HASHING 12346
#define ARRAY_SIZE 10
#define HASH_LENGTH 64
#define BUFFER_SIZE 1024

void fill_array_with_random(int *array, int size);
void run_sorting_function();
void run_hashing_function(const char *filename);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <choice> [filename]\n", argv[0]);
        fprintf(stderr, "Choice 1: Run sorting function\n");
        fprintf(stderr, "Choice 2: Run hashing function\n");
        exit(1);
    }

    int choice = atoi(argv[1]);

    if (choice == 1) {
        run_sorting_function();
    } else if (choice == 2) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s 2 <filename>\n", argv[0]);
            exit(1);
        }
        run_hashing_function(argv[2]);
    } else {
        fprintf(stderr, "Invalid choice. Use 1 for sorting or 2 for hashing.\n");
        exit(1);
    }

    return 0;
}

void run_sorting_function() {
    int sockfd;
    struct sockaddr_in serv_addr;
    int array[ARRAY_SIZE];
    int target;
    int received_array[ARRAY_SIZE];
    int count;

    // Fill the array with random numbers
    fill_array_with_random(array, ARRAY_SIZE);

    // Set a target number for searching
    target = rand() % 100; // Random target within the same range as array elements

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    // Server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_SORTING);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send array and target to server
    write(sockfd, array, sizeof(int) * ARRAY_SIZE);
    write(sockfd, &target, sizeof(int));

    // Receive sorted array and count from server
    read(sockfd, received_array, sizeof(int) * ARRAY_SIZE);
    read(sockfd, &count, sizeof(int));

    // Print the sorted array and the count of the target found
    printf("Sorted array: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", received_array[i]);
    }
    printf("\n");
    printf("Target %d found %d times\n", target, count);

    // Close the socket
    close(sockfd);
}

void run_hashing_function(const char *filename) {
    int sockfd, file_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT_HASHING);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send filename
    if (send(sockfd, filename, strlen(filename) + 1, 0) < 0) { // Include null-terminator
        perror("Send filename failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Open file
    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("File open failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Read from file and send to server
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

    // Close the write end of the socket to signal end of file transmission
    shutdown(sockfd, SHUT_WR);

    // Receive hash from server
    char hash[HASH_LENGTH + 1] = {0};
    if (recv(sockfd, hash, HASH_LENGTH, 0) < 0) {
        perror("Receive hash failed");
        close(file_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Received hash: %s\n", hash);

    // Clean up
    close(file_fd);
    close(sockfd);
}

void fill_array_with_random(int *array, int size) {
    srand(time(NULL)); // Initialize random seed
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100; // Random numbers between 0 and 99
    }
}
