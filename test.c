#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MAIN_PORT 12345
#define SORT_SEARCH_PORT 12346
#define HASH_PORT 12347
#define ARRAY_SIZE 10
#define BUFFER_SIZE 1024

void start_websocket_server(int port, void *(*handler)(void *));
void *gateway_handler(void *arg);
void *sort_search_handler(void *arg);
void *hash_handler(void *arg);
void sort_array(int *array, int size);
int binary_search(int *array, int left, int right, int target);

int main() {
    pid_t pid_sort_search, pid_hash;
    pid_sort_search = fork();
    if (pid_sort_search == 0) {
        start_websocket_server(SORT_SEARCH_PORT, sort_search_handler);
        exit(0);
    }

    pid_hash = fork();
    if (pid_hash == 0) {
        start_websocket_server(HASH_PORT, hash_handler);
        exit(0);
    }

    start_websocket_server(MAIN_PORT, gateway_handler);
    wait(NULL);
    return 0;
}

void start_websocket_server(int port, void *(*handler)(void *)) {
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
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    printf("Server is listening on port %d\n", port);

    while (1) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue;
        }

        if (handler != NULL) {
            pthread_t thread_id;
            int *new_sock = malloc(sizeof(int));
            *new_sock = newsockfd;
            if (pthread_create(&thread_id, NULL, handler, (void *)new_sock) != 0) {
                perror("could not create thread");
                free(new_sock);
            }
            pthread_detach(thread_id);
        }
    }

    close(sockfd);
}

void *gateway_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    free(socket_desc);
    int choice;
    read(sock, &choice, sizeof(choice));
    printf("THE CHOICE IS %d \n", choice);
    if (choice == 1) {
        
        // Connect to sorting/searching server and handle array and target
    } else if (choice == 2) {
        // Connect to hashing server and handle file data
    }

    close(sock);
    return NULL;
}

void *sort_search_handler(void *socket_desc) {
    // Handle sorting and searching here
    int sock = *(int *)socket_desc;
    free(socket_desc);

    int array[ARRAY_SIZE], target, index;
    read(sock, array, sizeof(array));
    read(sock, &target, sizeof(target));

    sort_array(array, ARRAY_SIZE);
    index = binary_search(array, 0, ARRAY_SIZE - 1, target);

    // Send back the sorted array and the index
    write(sock, array, sizeof(array));
    write(sock, &index, sizeof(index));

    close(sock);
    return NULL;
}

void *hash_handler(void *socket_desc) {
    // Handle file hashing here
    int sock = *(int *)socket_desc;
    free(socket_desc);

    char buffer[BUFFER_SIZE];
    read(sock, buffer, sizeof(buffer));

    // Perform hashing on buffer (pseudo-code)
    char hash_value[BUFFER_SIZE] = "hashed_value";  // Placeholder

    write(sock, hash_value, strlen(hash_value) + 1);
    close(sock);
    return NULL;
}

void sort_array(int *array, int size) {
    // Simple bubble sort (not efficient for large data)
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

int binary_search(int *array, int left, int right, int target) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (array[mid] == target)
            return mid;
        if (array[mid] < target)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;  // Element not found
}
