#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <node/openssl/sha.h>
#include "common.h"


void fill_array_with_random(int *array, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % ARRAY_SIZE;
    }
}

void read_array_from_file(int *arr, int size, char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    
    char buffer[ARRAY_SIZE * sizeof(int) * 2];
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        perror("Failed to read from file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char *token = strtok(buffer, " ");
    int index = 0;
    while (token != NULL && index < size) {
        arr[index++] = atoi(token);
        token = strtok(NULL, " ");
    }

    close(fd);
}

void write_to_file_if_empty(int *arr, int size, char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    struct stat statbuf;
    fstat(fd, &statbuf);

    if (statbuf.st_size == 0) {
        for (int i = 0; i < size; i++) {
            dprintf(fd, "%d ", arr[i]);
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename to hash> <key to search>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    int target = atoi(argv[2]);

    int *array = malloc(ARRAY_SIZE * sizeof(int));
    pid_t firstpid = fork();

    if(firstpid==0){
        char *filename = "rrr(1).json";
        clock_t start, end;
        double cpu_time_used;
        start = clock();
        hash_file(filename);
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Hashing process took %f seconds to execute \n", cpu_time_used);
        wait(NULL);
    }
    else{
        pid_t pid = fork();

        if (pid == 0) {
            clock_t start, end;
            double cpu_time_used;
            start = clock();
            fill_array_with_random(array, ARRAY_SIZE);
            write_to_file_if_empty(array, ARRAY_SIZE, "notsortedarray.txt");
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Random array filling process took %f seconds to execute\n", cpu_time_used);
            exit(0);
        } else {
            waitpid(pid, NULL, WUNTRACED);

            pid_t pid_sort = fork();
            
            if (pid_sort == 0) {
                clock_t start, end;
                double cpu_time_used;
                read_array_from_file(array, ARRAY_SIZE, "notsortedarray.txt");
                start = clock();
                if (fork() == 0) {
                    quicksort(array, 0, ARRAY_SIZE - 1);
                    write_to_file_if_empty(array, ARRAY_SIZE, "sortedarray.txt");
                    printf("Quicksort finished\n");
                    exit(0);
                }
                if (fork() == 0) {
                    insertionSort(array, ARRAY_SIZE);
                    write_to_file_if_empty(array, ARRAY_SIZE, "sortedarray.txt");
                    printf("Insertion sort finished\n");
                    exit(0);
                }
                if (fork() == 0) {
                    Merge_Sort(array, 0, ARRAY_SIZE - 1);
                    write_to_file_if_empty(array, ARRAY_SIZE, "sortedarray.txt");
                    printf("Merge sort finished\n");
                    exit(0);
                }
                if (fork() == 0) {
                    bubblesort(array);
                    write_to_file_if_empty(array, ARRAY_SIZE, "sortedarray.txt");
                    printf("Bubble sort finished\n");
                    exit(0);
                }
                while(wait(NULL) > 0);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Sorting processes took %f seconds to execute\n", cpu_time_used);
                exit(0);
            } else {
                waitpid(pid_sort, NULL, WUNTRACED);

                clock_t start, end;
                double cpu_time_used;
                start = clock();
                read_array_from_file(array, ARRAY_SIZE, "sortedarray.txt");
                
                if (fork() == 0) {
                    int index = binary_search(array, 0, ARRAY_SIZE - 1, target);
                    printf("Binary search FINISHED %d\n", index);
                    exit(0);
                }
                if (fork() == 0) {
                    int index = fibonacci_search(array, ARRAY_SIZE, target);
                    printf("Fibonacci search FINISHED %d\n", index);
                    exit(0);
                }
                if (fork() == 0) {
                    int index = jump_search(array, ARRAY_SIZE, target);
                    printf("Jump search FINISHED %d\n", index);
                    exit(0);
                }
                if (fork() == 0) {
                    int index = ternary_search(array,0, ARRAY_SIZE, target);
                    printf("Ternary search FINISHED %d\n", index);
                    exit(0);
                }
                while(wait(NULL) > 0);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Searching processes took %f seconds to execute\n", cpu_time_used);
                printf("HERE \n");
                if (remove("sortedarray.txt") != 0) {
                    perror("Error deleting sortedarray.txt");
                }
                if (remove("notsortedarray.txt") != 0) {
                    perror("Error deleting notsortedarray.txt");
                }

                free(array);
                exit(0);
            }
        }
    }
    
    return 0;
}
