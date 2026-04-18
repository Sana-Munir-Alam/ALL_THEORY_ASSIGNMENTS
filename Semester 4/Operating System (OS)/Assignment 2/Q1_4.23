#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Thread function: prints all primes <= limit */
void *print_primes(void *param) {
    int limit = *((int *)param);

    printf("Prime numbers up to %d:\n", limit);
    for (int num = 2; num <= limit; num++) {
        int is_prime = 1;
        for (int i = 2; i * i <= num; i++) {
            if (num % i == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime)
            printf("%d ", num);
    }
    printf("\n");
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int limit = atoi(argv[1]);
    if (limit < 2) {
        printf("Please enter a number >= 2.\n");
        return 1;
    }

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    /* Create the worker thread */
    pthread_create(&tid, &attr, print_primes, &limit);

    /* Wait for thread to finish */
    pthread_join(tid, NULL);

    return 0;
}
