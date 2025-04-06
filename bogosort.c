#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pcg_basic.h"

uint64_t bogosort(int *array, int length);
void shuffle(pcg32_random_t *rng, int *array, int length);
bool sorted(int *array, int length, bool ascending);
bool ascending_comparison(int x, int y);
bool descending_comparison(int x, int y);

#define INVALID_USE_MESSAGE "./bogosort [ARRAY LENGTH] -t [NUMBER OF THREADS]\n"

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        printf(INVALID_USE_MESSAGE);
        return 1;
    }

    if (argv[2] != "-t") {
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

    uint64_t tries = bogosort(initial_array, array_length);
    printf("Sorted in %llu tries\n", tries);

    return 0;
}

uint64_t bogosort(int *array, int length) {
    pcg32_random_t seed;
    pcg32_srandom_r(&seed, time(NULL) ^ (intptr_t)&printf, (intptr_t)&length);
    uint64_t tries = 0;

    while (true) {
        if (sorted(array, length, true)) {
            return tries;
        }

        shuffle(&seed, array, length);
        tries++;
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