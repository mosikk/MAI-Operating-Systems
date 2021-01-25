#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// scan a string with unknown length
char* get_string() {
    int len = 0, capacity = 10;
    char* s = (char*)malloc(10 * sizeof(char));
    if (s == NULL) {
        perror("Can't read a string1");
        exit(6);
    }

    char c;
    do {
        c = getchar();
        if (c == EOF) {
            close(0);
            exit(0);
        }
        s[len++] = c;
        if (len == capacity) {
            capacity *= 2;
            s = (char*)realloc(s, capacity * sizeof(char));
            if (s == NULL) {
                perror("Can't read a string2");
                exit(6);
            }
        }
    } while  (c != '\0');
    s[len] = 0;
    return s;
}

int main(int argc, char* argv[]) {
    while (1) {
        char* str = get_string();
        reverse_string(str);
        printf("%s\n", str);
        fflush(stdout);
    }
}