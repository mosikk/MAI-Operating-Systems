#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// writes size of str and str to pipe fd
void to_pipe(int* fd, char* str) {
    int i = 0;
    char c;
    do {
        c = str[i++];
        if (write(fd[1], &c, sizeof(char)) < 0) {
            perror("Can't write to the pipe");
            exit(4);
        }
    } while (c != '\0');
}

// scan a string with unknown length
char* get_string() {
    int len = 0, capacity = 10;
    char* s = (char*)malloc(10 * sizeof(char));
    if (s == NULL) {
        perror("Can't read a string");
        exit(6);
    }

    char c;
    while ((c = getchar()) != '\n') {
        s[len++] = c;
        if (c == EOF) {
            break;
        }
        if (len == capacity) {
            capacity *= 2;
            s = (char*)realloc(s, capacity * sizeof(char));
            if (s == NULL) {
                perror("Can't read a string");
                exit(6);
            }
        }
    };
    s[len] = '\0';
    return s;
}

int main() {
    srand(time(NULL));

    // creating files for child processes
    printf("Enter file's name for chiild process 1: ");
    char* file1_name = get_string();

    printf("Enter file's name for chiild process 2: ");
    char* file2_name = get_string();
    
    int file1 = open(file1_name, O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
    int file2 = open(file2_name, O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
    if (file1 < 0 || file2 < 0) {
        perror("Can't open file");
        exit(1);
    }

    // creating pipes for child processes
    int fd1[2];
    int fd2[2];
    // fd[0] - read, fd[1] - write
    if (pipe(fd1) < 0 || pipe(fd2) < 0) {
        perror("Can't create pipe");
        exit(2);
    }

    // creating child processes
    int pid1 = fork();
    if (pid1 < 0) {
        perror("Can't create child process");
        exit(3);
    }

    if (pid1 > 0) { // parent
        int pid2 = fork();
        if (pid2 < 0) {
            perror("Can't create child process");
            exit(3);
        }

        if (pid2 > 0) { // parent
            // close useless file descriptors
            close(fd1[0]);
            close(fd2[0]);
            
            while (1) {
                char* s = get_string();

                if (rand() % 100 + 1 <= 80) {
                    to_pipe(fd1, s);
                    if (s[0] == EOF) {
                        to_pipe(fd2, s);
                        break;
                    }
                }
                else {
                    to_pipe(fd2, s);
                    if (s[0] == EOF) {
                        to_pipe(fd1, s);
                        break;
                    }
                }
            }

            close(fd1[1]);
            close(fd2[1]);
        }
        else { // child2
            // close useless file descriptors
            close(fd1[0]);
            close(fd1[1]);
            close(fd2[1]);

            // redirecting standart input and output for child processes
            if (dup2(fd2[0], STDIN_FILENO) < 0) {
                perror("Can't redirect stdin for child process");
                exit(5);
            };
            if (dup2(file2, STDOUT_FILENO) < 0) {
                perror("Can't redirect stdout for child process");
                exit(5);
            }
            execl("child", NULL, NULL);

            // it won't go here if child executes
            perror("Can't execute child process");
            exit(6);
        }
    }
    else { // child1
        // close useless file descriptors
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);

        // redirecting standart input and output for child processes
        if (dup2(fd1[0], STDIN_FILENO) < 0) {
            perror("Can't redirect stdin for child process");
            exit(5);
        }
        if (dup2(file1, STDOUT_FILENO) < 0) {
            perror("Can't redirect stdout for child process");
            exit(5);
        }
        execl("child", NULL, NULL);

        // it won't go here if child executes
        perror("Can't execute child process");
        exit(6);
    }
}