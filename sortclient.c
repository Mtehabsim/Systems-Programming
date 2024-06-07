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


void fill_array_with_random(int *array, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % size;
    }
}


int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    int array[ARRAY_SIZE];
    int target;
    int received_array[ARRAY_SIZE];
    int count;

    fill_array_with_random(array, ARRAY_SIZE);

    target = rand() % ARRAY_SIZE;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    write(sockfd, array, sizeof(int) * ARRAY_SIZE);
    write(sockfd, &target, sizeof(int));

    read(sockfd, received_array, sizeof(int) * ARRAY_SIZE);
    read(sockfd, &count, sizeof(int));

    printf("Sorted array: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", received_array[i]);
    }
    printf("\n");
    printf("Target %d found %d times\n", target, count);

    close(sockfd);
    return 0;
}
