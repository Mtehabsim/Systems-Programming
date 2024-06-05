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
#include <pthread.h>
#include <node/openssl/sha.h>

#define ARRAY_SIZE 1000
#define MAX_HASHMAP_SIZE 1024
#define HASH_FILE "hash_data.txt"
#define SHA256_DIGEST_LENGTH 32
pthread_mutex_t array_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int* array;
    int target;
} SearchThreadData;
void load_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int *count) {
    int file = open(HASH_FILE, O_RDONLY);
    if (file != -1) {
        char line[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
            line[bytes_read] = '\0'; // end of file
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

void write_to_pipe(int *array, int size, int pipe_fd) {
    write(pipe_fd, array, size * sizeof(int));
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
    pthread_mutex_lock(&array_mutex);
    for (x = 0; x < num - 1; x++) {
        for (y = 0; y < num - x - 1; y++) {
            if (arr[y] > arr[y + 1]) {
                temp = arr[y];
                arr[y] = arr[y + 1];
                arr[y + 1] = temp;
            }
        }
    }
    pthread_mutex_unlock(&array_mutex);
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
    pthread_mutex_lock(&array_mutex);
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
    pthread_mutex_unlock(&array_mutex);
}

void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int pivot = arr[high];
        int i = (low - 1);
        for (int j = low; j < high; j++) {
            if (arr[j] < pivot) {
                i++;
                int temp = arr[i];
                pthread_mutex_lock(&array_mutex);
                arr[i] = arr[j];
                arr[j] = temp;
                pthread_mutex_unlock(&array_mutex);
            }
        }
        int temp = arr[i + 1];
        pthread_mutex_lock(&array_mutex);
        arr[i + 1] = arr[high];
        arr[high] = temp;
        pthread_mutex_unlock(&array_mutex);
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

void Merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int size1 = mid - left + 1;
    int size2 = right - mid;

    int Left[size1], Right[size2];
    pthread_mutex_lock(&array_mutex);
    for (i = 0; i < size1; i++)
        Left[i] = arr[left + i];
    for (j = 0; j < size2; j++)
        Right[j] = arr[mid + 1 + j];
    i = 0;
    j = 0;
    k = left;
    while (i < size1 && j < size2) {
        if (Left[i] <= Right[j]) {
            arr[k] = Left[i];
            i++;
        } else {
            arr[k] = Right[j];
            j++;
        }
        k++;
    }

    while (i < size1) {
        arr[k] = Left[i];
        i++;
        k++;
    }

    while (j < size2) {
        arr[k] = Right[j];
        j++;
        k++;
    }
    pthread_mutex_unlock(&array_mutex);
}

void Merge_Sort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        Merge_Sort(arr, left, mid);
        Merge_Sort(arr, mid + 1, right);
        Merge(arr, left, mid, right);
    }
}

int count_frequency(int arr[], int n, int index, int x) {
    if (index == -1) return 0;

    int count = 1;
    int left = index - 1;
    int right = index + 1;

    while (left >= 0 && arr[left] == x) {
        count++;
        left--;
    }

    while (right < n && arr[right] == x) {
        count++;
        right++;
    }

    return count;
}

void* jump_search_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    SearchThreadData* data = (SearchThreadData*)args;
    int index = jump_search(data->array, ARRAY_SIZE, data->target);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Jump search FINISHED target found %d times. Time taken: %f seconds\n", count_frequency(data->array, ARRAY_SIZE, index, data->target), cpu_time_used);
    pthread_exit(NULL);
}

void* ternary_search_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    SearchThreadData* data = (SearchThreadData*)args;
    int index = ternary_search(data->array, 0, ARRAY_SIZE - 1, data->target);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Ternary search FINISHED target found %d times. Time taken: %f seconds\n", count_frequency(data->array, ARRAY_SIZE, index, data->target), cpu_time_used);
    pthread_exit(NULL);
}

void* quicksort_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    int* array = (int*)args;
    quicksort(array, 0, ARRAY_SIZE - 1);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Quicksort FINISHED. Time taken: %f seconds\n", cpu_time_used);
    pthread_exit(NULL);
}

void* insertion_sort_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    int* array = (int*)args;
    insertionSort(array, ARRAY_SIZE);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Insertion sort FINISHED. Time taken: %f seconds\n", cpu_time_used);
    pthread_exit(NULL);
}

void* merge_sort_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    int* array = (int*)args;
    Merge_Sort(array, 0, ARRAY_SIZE - 1);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Merge sort FINISHED. Time taken: %f seconds\n", cpu_time_used);
    pthread_exit(NULL);
}

void* bubble_sort_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    int* array = (int*)args;
    bubblesort(array);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Bubble sort FINISHED. Time taken: %f seconds\n", cpu_time_used);
    pthread_exit(NULL);
}

void* binary_search_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    SearchThreadData* data = (SearchThreadData*)args;
    int index = binary_search(data->array, 0, ARRAY_SIZE - 1, data->target);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Binary search FINISHED target found %d times. Time taken: %f seconds\n", count_frequency(data->array, ARRAY_SIZE, index, data->target), cpu_time_used);
    pthread_exit(NULL);
}

void* fibonacci_search_thread(void* args) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    SearchThreadData* data = (SearchThreadData*)args;
    int index = fibonacci_search(data->array, ARRAY_SIZE, data->target);
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Fibonacci search FINISHED target found %d times. Time taken: %f seconds\n", count_frequency(data->array, ARRAY_SIZE, index, data->target), cpu_time_used);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename to hash> <key to search>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    int target = atoi(argv[2]);

    int *array = malloc(ARRAY_SIZE * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

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
    }else{
        clock_t start, end;
        double cpu_time_used;
        start = clock();

        fill_array_with_random(array, ARRAY_SIZE);
        pthread_t sort_threads[4], search_threads[4];
    
        pthread_create(&sort_threads[0], NULL, quicksort_thread, (void*)array);
        pthread_create(&sort_threads[1], NULL, insertion_sort_thread, (void*)array);
        pthread_create(&sort_threads[2], NULL, merge_sort_thread, (void*)array);
        pthread_create(&sort_threads[3], NULL, bubble_sort_thread, (void*)array);

        for (int i = 0; i < 4; i++) {
            pthread_join(sort_threads[i], NULL);
        }

        SearchThreadData searchdata = {array, target};

        pthread_create(&search_threads[0], NULL, binary_search_thread, (void*)&searchdata);
        pthread_create(&search_threads[1], NULL, fibonacci_search_thread, (void*)&searchdata);
        pthread_create(&search_threads[2], NULL, jump_search_thread, (void*)&searchdata);
        pthread_create(&search_threads[3], NULL, ternary_search_thread, (void*)&searchdata);

        for (int i = 0; i < 4; i++) {
            pthread_join(search_threads[i], NULL);
        }
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Sort and search process took %f seconds to execute \n", cpu_time_used);
        free(array);
    }
    
    return 0;
}