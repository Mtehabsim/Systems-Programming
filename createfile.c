#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int file_desc;
    const char *file_name = "example.txt";
    const char *data = "test test 1 2 3 \n";

    file_desc = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file_desc < 0) {
        perror("Error opening file");
        return 1;
    }

    if (write(file_desc, data, strlen(data)) < 0) {
        perror("Error writing to file");
        close(file_desc);
        return 1;
    }

    if (close(file_desc) < 0) {
        perror("Error closing file");
        return 1;
    }

    printf("Data written to %s successfully.\n", file_name);
    return 0;
}