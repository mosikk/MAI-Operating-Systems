#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MAP_SIZE 4096

// files for mapping
char* file1_name = "file1_mapped";
char* file2_name = "file2_mapped";

// empty string as a signal
char empty = 1;
char* empty_string = &empty;

void reverse_string(char *str) {
    int length = strlen(str);
    char *front = str;
    char *back = str + length - 1;

    while (front < back) {
        char tmp = *front;
        *front = *back;
        *back = tmp;
        ++front;
        --back;
    }
}

int main(int argc, char* argv[]) {
    char* file_name;
    if (argv[0][0] == '1') {
        file_name = file1_name;
    }
    else if (argv[0][0] == '2') {
        file_name = file2_name;
    }
    else {
        perror("Unknown file");
        exit(8);
    }
    // opening a file for mapping
    int fd = open(file_name, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
    if (fd < 0) {
        perror("Can't open file");
        exit(1);
    }

    // mapping file
    char* file = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file == MAP_FAILED) {
        perror("Can't map a file");
        exit(2);
    }

    while (1) {
        // wainting for a string
        while (strcmp(file, empty_string) == 0) {}

        // terminating if Ctrl+D was pressed
        if (file[0] == EOF) {
            if (munmap(file, MAP_SIZE) < 0) {
                perror("Can't unmap file");
                exit(4);
            }
            exit(0);
        }

        char* string = (char*)malloc(strlen(file) * sizeof(char));
        strcpy(string, file);
        reverse_string(string);
        printf("%s\n", string);
        fflush(stdout);
        strcpy(file, empty_string);
        free(string);
    }
}