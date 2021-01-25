#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

char* sieve;
long long num;

// thread function - marking numbers that are multiple of i
void* sieve_step(void* i_void) {
    long long i = *(long long*)i_void;
    for (long long j = i * i; j <= num; j += i) {
        sieve[j] = 1;
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Syntax: ./main Number_of_threads\n");
        exit(1);
    }

    int threads_num = atoi(argv[1]);

    pthread_t* threads = (pthread_t*)calloc(threads_num, sizeof(pthread_t));
    if (threads == NULL) {
        printf("Can't allocate space for threads\n");
        exit(2);
    }

    // array for arguments which will be passed into thread function
    long long* args = (long long*)malloc(threads_num * sizeof(long long));
    if (args == NULL) {
        printf("Can't create an array for arguments for threads\n");
        exit(3);
    }

    printf("Enter a number you want to check: ");
    scanf("%lld", &num);

    // creating array filled with 0 for sieve
    // 0 - prime number, 1 - nonprime number
    sieve = (char*)calloc((num + 1), sizeof(char));
    if (sieve == NULL) {
        printf("Can't create an array for sieve\n");
        exit(3);
    }

    // marking numbers which are not prime by defenition
    sieve[0] = 1;
    sieve[1] = 1;

    int cur_thread = 0; // id of current thread
    for (long long i = 2; i * i <= num; ++i) {
        if (sieve[i] == 1) { // skipping not prime numbers
            continue;
        }
        if (cur_thread >= threads_num) {
            // we should wait while necessery thread is working
            pthread_join(threads[cur_thread % threads_num], NULL);
        }

        args[cur_thread % threads_num] = i; // copying argument for thread function to a special array
        pthread_create(&threads[cur_thread % threads_num], NULL, sieve_step, &args[cur_thread % threads_num]);
        ++cur_thread;
    }

    // waiting for all threads
    for (int i = 0; i < threads_num; ++i) {
        pthread_join(threads[i], NULL);
    }

    if (sieve[num] == 1) {
        printf("%lld is not a prime number\n", num);
    }
    else {
        printf("%lld is a prime number\n", num);
    }

    free(sieve);
    free(threads);
    free(args);
}
