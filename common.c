#include "common.h"

int min(int a, int b) {
    return a < b ? a : b;
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

int binary_search(int arr[], int l, int r, int x) {
    while (l <= r) {
        int m = l + (r - l) / 2;
        if (arr[m] == x)
            return m;
        if (arr[m] < x)
            l = m + 1;
        else
            r = m - 1;
    }
    return -1;
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

void load_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int *count) {
    FILE *file = fopen(HASH_FILE, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';  // Remove newline character

        char *filename = strtok(line, " ");
        char *hash = strtok(NULL, " ");
        if (filename && hash) {
            if (*count >= MAX_HASHMAP_SIZE) {
                fprintf(stderr, "Error: Hashmap is full\n");
                break;
            }
            hashmap[*count][0] = strdup(filename);
            hashmap[*count][1] = strdup(hash);
            (*count)++;
        }
    }
    fclose(file);
}

void save_hash_map(char *hashmap[MAX_HASHMAP_SIZE][2], int count) {
    int file = open(HASH_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file == -1) {
        perror("Failed to open hash file for writing");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (dprintf(file, "%s %s\n", hashmap[i][0], hashmap[i][1]) < 0) {
            perror("Failed to write hash to file");
        }
    }
    close(file);
}

char* hash_file(char *filename) {
    char *hashmap[MAX_HASHMAP_SIZE][2] = {0};
    int count = 0;
    load_hash_map(hashmap, &count);

    for (int i = 0; i < count; i++) {
        if (strcmp(hashmap[i][0], filename) == 0) {
            return hashmap[i][1]; // Return cached hash
        }
    }

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("Cannot open file to hash");
        return NULL;
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

    char *hex_output = malloc(65 * sizeof(char));
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_output + i * 2, "%02x", sha256_digest[i]);
    }
    hex_output[64] = '\0';
    close(file);

    if (count < MAX_HASHMAP_SIZE) {
        hashmap[count][0] = strdup(filename);
        hashmap[count][1] = strdup(hex_output);
        count++;
    }
    save_hash_map(hashmap, count);

    // Clean up memory for loaded hashmap
    for (int i = 0; i < count; i++) {
        free(hashmap[i][0]);
        free(hashmap[i][1]);
    }

    return hex_output;
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