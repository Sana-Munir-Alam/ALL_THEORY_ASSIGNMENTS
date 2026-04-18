#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Global variables - set by worker threads, read by parent */
double average;
int minimum;
int maximum;

/* Shared data structure passed to all threads */
typedef struct {
    int *numbers;
    int count;
} ThreadData;

/* Thread 1: Calculate average */
void *calc_average(void *param) {
    ThreadData *data = (ThreadData *)param;
    double sum = 0.0;
    for (int i = 0; i < data->count; i++)
        sum += data->numbers[i];
    average = sum / data->count;
    pthread_exit(0);
}

/* Thread 2: Calculate maximum */
void *calc_maximum(void *param) {
    ThreadData *data = (ThreadData *)param;
    maximum = data->numbers[0];
    for (int i = 1; i < data->count; i++)
        if (data->numbers[i] > maximum)
            maximum = data->numbers[i];
    pthread_exit(0);
}

/* Thread 3: Calculate minimum */
void *calc_minimum(void *param) {
    ThreadData *data = (ThreadData *)param;
    minimum = data->numbers[0];
    for (int i = 1; i < data->count; i++)
        if (data->numbers[i] < minimum)
            minimum = data->numbers[i];
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <num1> <num2> ...\n", argv[0]);
        return 1;
    }

    int count = argc - 1;
    int *numbers = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++)
        numbers[i] = atoi(argv[i + 1]);

    ThreadData data = { numbers, count };

    pthread_t tid1, tid2, tid3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    /* Create the three worker threads */
    pthread_create(&tid1, &attr, calc_average, &data);
    pthread_create(&tid2, &attr, calc_maximum, &data);
    pthread_create(&tid3, &attr, calc_minimum, &data);

    /* Wait for all threads to complete */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    /* Parent thread outputs the results */
    printf("The average value is %.0f\n", average);
    printf("The minimum value is %d\n", minimum);
    printf("The maximum value is %d\n", maximum);

    free(numbers);
    return 0;
}
