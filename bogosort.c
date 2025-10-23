#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pcg_basic.h"

#define INVALID_USE_MESSAGE "./bogosort [ARRAY LENGTH] -t [NUMBER OF THREADS]\n"

typedef struct bogosort_args {
    int *array;
    int length;
} bogosort_args;

void *bogosort_wrapper(void *args);
uint64_t *bogosort(bogosort_args *args);
void shuffle(pcg32_random_t *rng, int *array, int length);
bool sorted(int *array, int length, bool ascending);
bool ascending_comparison(int x, int y);
bool descending_comparison(int x, int y);
int number_of_digits(uint64_t x);

bool SORTED = false;

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }

    if (strcasecmp(argv[2], "-t")) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }

    int array_length = strtod(argv[1], NULL);
    if (array_length == 0) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }

    int number_of_threads = strtod(argv[3], NULL);
    if (number_of_threads == 0) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }

    int *initial_array = malloc(array_length * sizeof(int));
    if (initial_array == NULL) {
        printf("Can't allocate memory\n");
        return 1;
    }

    for (int i = 0; i < array_length; i++) {
        initial_array[i] = i + 1;
    }

    pcg32_random_t s;
    pcg32_srandom_r(&s, time(NULL) ^ (intptr_t)&printf,
                    (intptr_t)&array_length);
    shuffle(&s, initial_array, array_length);

    printf("Original Array: [");
    for (int i = 0; i < array_length; i++) {
        printf("%i", initial_array[i]);
        if ((i + 1) != array_length) {
            printf(" ");
        }
    }
    printf("]\n\n");

    // One list per thread
    int *array_matrix[number_of_threads];
    for (int i = 0; i < number_of_threads; i++) {
        array_matrix[i] = malloc(array_length * sizeof(int));
        if (array_matrix[i] == NULL) {
            printf("Can't allocate memory\n");
            return 1;
        }
        for (int j = 0; j < array_length; j++) {
            array_matrix[i][j] = initial_array[j];
        }
    }

    free(initial_array); // We done using this

    pthread_t thread_ids[number_of_threads];
    bogosort_args *func_args[number_of_threads];

    for (int i = 0; i < number_of_threads; i++) {
        func_args[i] = malloc(sizeof(bogosort_args));
        if (func_args[i] == NULL) {
            printf("Can't allocate memory\n");
            return 1;
        }

        func_args[i]->array = array_matrix[i];
        func_args[i]->length = array_length;
        pthread_create(&thread_ids[i], NULL, &bogosort_wrapper, func_args[i]);
    }

    // Wait for the sorting to complete
    while (!SORTED) {
        continue;
    }

    uint64_t *tries_per_thread[number_of_threads];
    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(thread_ids[i], (void **)&tries_per_thread[i]);
    }

    unsigned long long total_tries = 0;
    for (int i = 0; i < number_of_threads; i++) {
        total_tries += (tries_per_thread[i] == NULL) ? 0 : *tries_per_thread[i];
    }

    printf("Sorted!\n\n");

    printf("Arrays in each Thread:\n");
    for (int i = 0; i < number_of_threads; i++) {
        printf("Thread %llu: [", (unsigned long long)thread_ids[i]);
        for (int j = 0; j < array_length; j++) {
            printf("%i", array_matrix[i][j]);
            if ((j + 1) != array_length) {
                printf(" ");
            }
        }
        printf("]\n");
    }

    int tries_size = number_of_digits(total_tries);

    int tries_str_length =
        ((tries_size % 3) == 0 ? -1 : 0) + (tries_size / 3) + tries_size;

    char tries_str[tries_str_length + 1];
    tries_str[tries_str_length] = '\0';

    char buffer[tries_size + 1];
    snprintf(buffer, ((tries_size + 1) * sizeof(char)), "%llu", total_tries);

    int offset = 0;
    int comma_count = 0;
    for (int i = 1; i <= tries_str_length; i++) {
        char digit = buffer[tries_size - (i - offset)];
        comma_count++;

        if (comma_count == 4) {
            digit = ',';
            comma_count = 0;
            offset++;
        }

        tries_str[tries_str_length - i] = digit;
    }

    printf("\nSorted in %s tries\n", tries_str);

    for (int i = 0; i < number_of_threads; i++) {
        free(func_args[i]);
        free(array_matrix[i]);
        free(tries_per_thread[i]);
    }

    return 0;
}

void *bogosort_wrapper(void *args) {
    pthread_exit(bogosort(args));
    return NULL;
}

uint64_t *bogosort(bogosort_args *args) {
    int *array = args->array;
    int length = args->length;

    pcg32_random_t seed;
    pcg32_srandom_r(&seed, time(NULL) ^ (intptr_t)&printf, (intptr_t)&length);
    uint64_t *tries = malloc(sizeof(uint64_t));
    if (tries == NULL) {
        return NULL;
    }
    *tries = 0;

    while (true) {
        if (SORTED || sorted(array, length, true)) {
            SORTED = true;
            break;
        }

        shuffle(&seed, array, length);
        (*tries)++;
    }

    return tries;
}

// In place shuffling of an array
void shuffle(pcg32_random_t *rng, int *array, int length) {
    for (int i = 0; i < length; i++) {
        int swapped_index = pcg32_boundedrand_r(rng, length);
        int tmp = array[i];
        array[i] = array[swapped_index];
        array[swapped_index] = tmp;
    }

    return;
}

bool sorted(int *array, int length, bool ascending) {
    bool (*comparison_func)(int, int) =
        ascending ? &ascending_comparison : &descending_comparison;
    for (int i = 0; i < length - 1; i++) {
        if (!comparison_func(array[i], array[i + 1])) {
            return false;
        }
    }

    return true;
}

bool ascending_comparison(int x, int y) { return x < y; }

bool descending_comparison(int x, int y) { return x > y; }

int number_of_digits(uint64_t x) {
    if (x == 0) {
        return 1;
    }

    int digits = 0;

    while (x != 0) {
        x = x / 10;
        digits++;
    }

    return digits;
}
