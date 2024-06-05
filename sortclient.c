#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define ARRAY_SIZE 10

void fill_array_with_random(int *array, int size);

int main() {
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
        return -1;
    }

    // Server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
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
    return 0;
}

void fill_array_with_random(int *array, int size) {
    srand(time(NULL)); // Initialize random seed
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100; // Random numbers between 0 and 99
    }
}
