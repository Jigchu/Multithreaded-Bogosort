#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pcg_basic.h"

#define INVALID_USE_MESSAGE "./bogosort [ARRAY LENGTH] -t [NUMBER OF THREADS]\n"

void *bogosort_wrapper(void *args);
uint64_t *bogosort(int **args);
void shuffle(pcg32_random_t *rng, int *array, int length);
bool sorted(int *array, int length, bool ascending);
bool ascending_comparison(int x, int y);
bool descending_comparison(int x, int y);

struct bogosort_args {
    int *array;
    int length;
};

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
    
    int array_length = atoi(argv[1]);
    if (array_length == 0) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }
    
    int number_of_threads = atoi(argv[3]);
    if (number_of_threads == 0) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }
    
    int *initial_array = malloc(array_length * sizeof(int));
    if (initial_array == NULL) {
        printf("Can't allocate memory");
        return 1;
    }
    
    for (int i = 0; i < array_length; i++) {
        initial_array[i] = i + 1;
    }
    
    pcg32_random_t s;
    pcg32_srandom_r(&s, time(NULL) ^ (intptr_t)&printf, (intptr_t)&array_length);
    shuffle(&s, initial_array, array_length);
    
    // One list per thread
    int *array_matrix[number_of_threads];
    for (int i = 0; i < number_of_threads; i++) {
        array_matrix[i] = malloc(array_length * sizeof(int));
        if (array_matrix[i] == NULL) {
            printf("Can't allocate memory");
            return 1;
        }
        
        for (int j = 0; j < array_length; j++) {
            array_matrix[i][j] = initial_array[j];
        }
    }
    
    pthread_t thread_ids[number_of_threads];

    for (int i = 0; i < number_of_threads; i++) {
        int **func_args = malloc(sizeof(int *) * 2);
        if (func_args == NULL) {
            printf("Can't allocate memory");
            return 1;
        }
        func_args[0] = array_matrix[i];
        func_args[1] = &array_length;

        thread_ids[i] = pthread_create(&thread_ids[i], NULL, &bogosort_wrapper, func_args);
    }

    // Wait for the sorting to complete
    while (!SORTED) {
        continue;
    }

    uint64_t tries_per_thread[number_of_threads];
    for (int i = 0; i < number_of_threads; i++) {
        uint64_t *retval;
        pthread_join(thread_ids[i], (void **) &retval);
        tries_per_thread[i] = retval == NULL ? 0 : *retval;
    }

    uint64_t total_tries;
    for (int i = 0; i < number_of_threads; i++) {
        total_tries += tries_per_thread[i];
    }

    printf("Original Array: [");
    for (int i = 0; i < array_length; i++) {
        printf("%i", initial_array[i]);
        if ((i+1) != array_length) {
            printf(" ");
        }
    }
    printf("]\n");
    printf("Sorted in %llu tries\n", total_tries);

    return 0;
}

void *bogosort_wrapper(void *args) {
    pthread_exit(bogosort(args));
    return NULL;
}

uint64_t *bogosort(int **args) {
    int *array = args[0];
    int length = args[1][0];

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
            return tries;
        }

        shuffle(&seed, array, length);
        (*tries)++;
    }
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
    bool (*comparison_func) (int, int) = ascending ? &ascending_comparison : &descending_comparison;
    for (int i = 0; i < length-1; i++) {
        if (!comparison_func(array[i], array[i+1])) {
            return false;
        }
    }

    return true;
}

bool ascending_comparison(int x, int y) {
    return x < y;
}

bool descending_comparison(int x, int y) {
    return x > y;
}