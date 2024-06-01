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

#define ARRAY_SIZE 1000
#define MAX_HASHMAP_SIZE 1024
#define HASH_FILE "hash_data.txt"
#define SHA256_DIGEST_LENGTH 32

void load_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int *count) {
    int file = open(HASH_FILE, O_RDONLY);
    if (file != -1) {
        char line[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
            line[bytes_read] = '\0'; // End of file
            char *filename = strtok(line, " ");
            char *hash = strtok(NULL, "\n");
            if (filename && hash) {
                hashmap[*count][0] = strdup(filename);
                hashmap[*count][1] = strdup(hash);
                (*count)++;
            }
        }
        close(file);
    }
}

void save_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int count) {
    int file = open(HASH_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file != -1) {
        for (int i = 0; i < count; i++) {
            dprintf(file, "%s %s\n", hashmap[i][0], hashmap[i][1]);
        }
        close(file);
    }
}

void hash_file(char *filename) {
    char *hashmap[MAX_HASHMAP_SIZE][2] = {0};
    int count = 0;
    load_hash_map(hashmap, &count);

    for (int i = 0; i < count; i++) {
        if (strcmp(hashmap[i][0], filename) == 0) {
            printf("SHA-256 Hash from cache: %s\n", hashmap[i][1]);
            return;
        }
    }

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }

    unsigned char data[1024];
    int bytes;
    unsigned char sha256_digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_context;

    SHA256_Init(&sha256_context);
    while ((bytes = read(file, data, sizeof(data))) > 0) {
        SHA256_Update(&sha256_context, data, bytes);
    }
    SHA256_Final(sha256_digest, &sha256_context);

    char hex_output[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_output + i * 2, "%02x", sha256_digest[i]);
    }
    hex_output[64] = '\0';

    printf("SHA-256 Hash: %s\n", hex_output);
    close(file);

    if (count < MAX_HASHMAP_SIZE) {
        hashmap[count][0] = strdup(filename);
        hashmap[count][1] = strdup(hex_output);
        count++;
    }
    save_hash_map(hashmap, count);
}


void fill_array_with_random(int *array, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % ARRAY_SIZE;
    }
}

int linear_search(int arr[], int n, int x) {
    for (int i = 0; i < n; i++) {
        if (arr[i] == x)
            return i;
    }
    return -1;
}

int min(int a, int b) {
    return a < b ? a : b;
}


int jump_search(int arr[], int n, int x) {
    int step = sqrt(n);
    int prev = 0;
    while (arr[min(step, n) - 1] < x) {
        prev = step;
        step += sqrt(n);
        if (prev >= n)
            return -1;
    }
    while (arr[prev] < x) {
        prev++;
        if (prev == min(step, n))
            return -1;
    }
    if (arr[prev] == x)
        return prev;
    return -1;
}

void bubblesort(int arr[]) {
    int num = ARRAY_SIZE;
    int temp, x, y;
    for (x = 0; x < num - 1; x++) {
        for (y = 0; y < num - x - 1; y++) {
            if (arr[y] > arr[y + 1]) {
                temp = arr[y];
                arr[y] = arr[y + 1];
                arr[y + 1] = temp;
            }
        }
    }
}

int binary_search(int arr[], int left, int right, int search_value) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == search_value) {
            return mid;
        } else if (arr[mid] < search_value) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}
void insertionSort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++) 
    {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) 
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int pivot = arr[high];
        int i = (low - 1);
        for (int j = low; j < high; j++) {
            if (arr[j] < pivot) {
                i++;
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        int temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;
        quicksort(arr, low, i);
        quicksort(arr, i + 2, high);
    }
}

int fibonacci_search(int arr[], int n, int x) {
    int fib2 = 0;
    int fib1 = 1;
    int fib = fib1 + fib2;
    while (fib < n) {
        fib2 = fib1;
        fib1 = fib;
        fib = fib1 + fib2;
    }
    int offset = -1;
    while (fib > 1) {
        int i = min(offset + fib2, n - 1);
        if (arr[i] < x) {
            fib = fib1;
            fib1 = fib2;
            fib2 = fib - fib1;
            offset = i;
        } else if (arr[i] > x) {
            fib = fib2;
            fib1 -= fib2;
            fib2 = fib - fib1;
        } else {
            return i;
        }
    }
    if (fib1 == 1 && arr[offset + 1] == x)
        return offset + 1;
    return -1;
}

void write_to_pipe(int *array, int size, int pipe_fd) {
    write(pipe_fd, array, size * sizeof(int));
}

int ternary_search(int arr[], int l, int r, int x) {
    if (r >= l) {
        int mid1 = l + (r - l) / 3;
        int mid2 = r - (r - l) / 3;
        if (arr[mid1] == x)
            return mid1;
        if (arr[mid2] == x)
            return mid2;
        if (arr[mid1] > x)
            return ternary_search(arr, l, mid1 - 1, x);
        if (arr[mid2] < x)
            return ternary_search(arr, mid2 + 1, r, x);
        return ternary_search(arr, mid1 + 1, mid2 - 1, x);
    }
    return -1;
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

void Merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int size1 = mid - left + 1;
    int size2 = right - mid;

    int Left[size1], Right[size2];

    for (i = 0; i < size1; i++)
        Left[i] = arr[left + i];
    for (j = 0; j < size2; j++)
        Right[j] = arr[mid + 1 + j];
    i = 0;
    j = 0;
    k = left;
    while (i < size1 && j < size2)
    {
        if (Left[i] <= Right[j])
        {
            arr[k] = Left[i];
            i++;
        }
        else
        {
            arr[k] = Right[j];
            j++;
        }
        k++;
    }

    while (i < size1)
    {
        arr[k] = Left[i];
        i++;
        k++;
    }

    while (j < size2)
    {
        arr[k] = Right[j];
        j++;
        k++;
    }
}
void Merge_Sort(int arr[], int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        Merge_Sort(arr, left, mid);
        Merge_Sort(arr, mid + 1, right);
        Merge(arr, left, mid, right);
    }
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
            int pipes[4][2];
		    for (int i = 0; i < 4; i++) {
			if (pipe(pipes[i]) == -1) {
			    perror("Pipe creation failed");
			    return 1;
			}
            }
            pid_t pid_sort = fork();
            
            if (pid_sort == 0) {
                clock_t start, end;
                double cpu_time_used;
                read_array_from_file(array, ARRAY_SIZE, "notsortedarray.txt");
                
                start = clock();
                
            pid_t pid;
            // int array[ARRAY_SIZE];
            
                if (fork() == 0) {
                    // for(int i =0;i<ARRAY_SIZE; i++){
                    //     printf("%d ", array[i]);
                    // }
                    quicksort(array, 0, ARRAY_SIZE - 1);
                    close(pipes[0][0]);
                    
            	    write_to_pipe(array, ARRAY_SIZE, pipes[0][1]);
            	     
                    printf("Quicksort finished\n");
                    exit(0);
                }
                if (fork() == 0) {
                    insertionSort(array, ARRAY_SIZE);
                    close(pipes[1][0]);
            	    write_to_pipe(array, ARRAY_SIZE, pipes[1][1]);
                    printf("Insertion sort finished\n");
                    exit(0);
                }
                if (fork() == 0) {
                    Merge_Sort(array, 0, ARRAY_SIZE - 1);
                    close(pipes[2][0]);
            	    write_to_pipe(array, ARRAY_SIZE, pipes[2][1]);
                    printf("Merge sort finished\n");
                    exit(0);
                }
                if (fork() == 0) {
                    bubblesort(array);
                    // close(pipes[3][0]);
            	    write_to_pipe(array, ARRAY_SIZE, pipes[3][1]);
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
                
                if (fork() == 0) {
                    read(pipes[0][0], array, ARRAY_SIZE * sizeof(int));
                    // for(int i =0;i<ARRAY_SIZE; i++){
                    //     printf("%d ", array[i]);
                    // }
                    int index = binary_search(array, 0, ARRAY_SIZE - 1, target);
                    printf("Binary search FINISHED %d\n", index);
                    exit(0);
                }
                if (fork() == 0) {
                    read(pipes[1][0], array, ARRAY_SIZE * sizeof(int));
                    int index = fibonacci_search(array, ARRAY_SIZE, target);
                    printf("Fibonacci search FINISHED %d\n", index);
                    exit(0);
                }
                if (fork() == 0) {
                    read(pipes[2][0], array, ARRAY_SIZE* sizeof(int));
                    int index = jump_search(array, ARRAY_SIZE, target);
                    printf("Jump search FINISHED %d\n", index);
                    exit(0);
                }
                if (fork() == 0) {
                    read(pipes[3][0], array, ARRAY_SIZE* sizeof(int));
                    int index = ternary_search(array,0, ARRAY_SIZE, target);
                    printf("Ternary search FINISHED %d\n", index);
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