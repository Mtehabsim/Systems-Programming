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

// #define ARRAY_SIZE 1000
#define MAX_HASHMAP_SIZE 1024
#define HASH_FILE "hash_data.txt"
#define SHA256_DIGEST_LENGTH 32


void fill_array_with_random(int *array, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % ARRAY_SIZE;
    }
}


void write_to_pipe(int *array, int size, int pipe_fd) {
    write(pipe_fd, array, size * sizeof(int));
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
            printf("Array not sorted : ");
            for(int i =0; i< ARRAY_SIZE; i++)
            printf("%d ", array[i]);
            write_to_file_if_empty(array, ARRAY_SIZE, "notsortedarray.txt");
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Random array filling process took %f seconds to execute\n", cpu_time_used);
            exit(0);
        } else {
            waitpid(pid, NULL, WUNTRACED);
            int pipes[4][2];
		    for (int i = 0; i < 4; i++) {
			if (pipe(pipes[i]) == -1) {
			    perror("Pipe creation failed");
			    return 1;
			}
            }
            pid_t pid_sort = fork();
            
            if (pid_sort == 0) {
                read_array_from_file(array, ARRAY_SIZE, "notsortedarray.txt");
                clock_t start, end;
                double cpu_time_used;
                start = clock();
                
            pid_t pid;
            
                if (fork() == 0) {
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    
                    quicksort(array, 0, ARRAY_SIZE - 1);
                    close(pipes[0][0]);
                    
            	    write_to_pipe(array, ARRAY_SIZE, pipes[0][1]);
            	     
                    
                    newend = clock();
                    printf("Quicksort finished Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    exit(0);
                }
                if (fork() == 0) {
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    
                    insertionSort(array, ARRAY_SIZE);
                    close(pipes[1][0]);
            	    write_to_pipe(array, ARRAY_SIZE, pipes[1][1]);
                    
                    
                    newend = clock();
                    printf("Insertion sort finished Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    exit(0);
                }
                if (fork() == 0) {
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    
                    Merge_Sort(array, 0, ARRAY_SIZE - 1);
                    close(pipes[2][0]);
            	    write_to_pipe(array, ARRAY_SIZE, pipes[2][1]);
                    
                    newend = clock();
                    printf("Merge sort finished Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    exit(0);
                }
                if (fork() == 0) {
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    
                    bubblesort(array);

                    close(pipes[3][0]);
            	    write_to_pipe(array, ARRAY_SIZE, pipes[3][1]);
                    newend = clock();
                    printf("Bubble sort finished Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
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
                
                if (fork() == 0) {
                    read(pipes[0][0], array, ARRAY_SIZE * sizeof(int));
                    printf("Array sorted : ");
                    for(int i =0; i< ARRAY_SIZE; i++)
                    printf("%d ", array[i]);
                    printf("\n");

                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    int index = binary_search(array, 0, ARRAY_SIZE - 1, target);
                    printf("Binary search FINISHED target found %d times\n", count_frequency(array, ARRAY_SIZE, index, target));
                    newend = clock();
                    printf("Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    
                    exit(0);
                }
                if (fork() == 0) {
                    read(pipes[1][0], array, ARRAY_SIZE * sizeof(int));
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    
                    int index = fibonacci_search(array, ARRAY_SIZE, target);
                    printf("Fibonacci search FINISHED target found %d times\n", count_frequency(array, ARRAY_SIZE, index, target));
                    newend = clock();
                    printf("Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    exit(0);
                }
                if (fork() == 0) {
                    read(pipes[2][0], array, ARRAY_SIZE* sizeof(int));
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    
                    int index = jump_search(array, ARRAY_SIZE, target);
                    printf("Jump search FINISHED target found %d times\n", count_frequency(array, ARRAY_SIZE, index, target));
                    newend = clock();
                    printf("Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    exit(0);
                }
                if (fork() == 0) {
                    read(pipes[3][0], array, ARRAY_SIZE* sizeof(int));
                    clock_t newstart, newend;
                    double new_cpu_time_used;
                    newstart = clock();
                    int index = ternary_search(array,0, ARRAY_SIZE, target);
                    printf("Ternary search FINISHED target found %d times\n", count_frequency(array, ARRAY_SIZE, index, target));
                    newend = clock();
                    printf("Took %f \n", ((double) (newend - newstart)) / CLOCKS_PER_SEC);
                    exit(0);
                }
                while(wait(NULL) > 0);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Searching processes took %f seconds to execute\n", cpu_time_used);
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