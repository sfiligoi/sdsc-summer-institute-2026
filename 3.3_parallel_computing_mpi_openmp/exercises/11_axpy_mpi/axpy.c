/*
 * Simple AXPY benchmark test.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

// Default values if no command line arguments are provided
#define DEFAULT_VECTOR_SIZE 1000000
#define DEFAULT_ITERATIONS 500


void one_axpy(int vector_size, float *Z, float *X, float *Y, float alpha) {
    // Main AXPY loop
    for (int i = 0; i < vector_size; i++) {
        Z[i] = alpha * X[i] + Y[i];
    }

}

int main(int argc, char *argv[]) {
    // TODO: Initialize MPI

    int vector_size = DEFAULT_VECTOR_SIZE;
    int iterations = DEFAULT_ITERATIONS;

    // Parse command line arguments if provided
    if (argc >= 2) {
        vector_size = atoi(argv[1]);
        if (vector_size <= 0) vector_size = DEFAULT_VECTOR_SIZE;
    }
    if (argc >= 3) {
        iterations = atoi(argv[2]);
        if (iterations <= 0) iterations = DEFAULT_ITERATIONS;
    }

    // TODO: Only deal with my own slice of the buffers

    // Allocate memory for vectors
    float *X = (float *)malloc(vector_size * sizeof(float));
    float *Y = (float *)malloc(vector_size * sizeof(float));
    float *Z = (float *)malloc(vector_size * sizeof(float));
    float alpha = 2.5f;

    if (X == NULL || Y == NULL || Z == NULL) {
        fprintf(stderr, "Memory allocation failed for vector size %d.\n", vector_size);
        return 1;
    }

    // Initialize vectors with dummy data
    // TODO Hint: While all C pointers start from 0,
    //            the first logical index used for initialization will not be zero in all processes
    for (int i = 0; i < vector_size; i++) {
        Z[i] = 0.0;
        X[i] = sin((float)i) * 0.5;
        Y[i] = cos((float)i) * 0.25;
    }

    // TODO: Only one process should print out
    //       Hint: You may want to distinguish between local and global buffer size

    double size_mb = (double)(vector_size * sizeof(float)) / (1024.0 * 1024.0);
    printf("Running AXPY benchmark...\n");
    printf("Vector Size: %d elements (~%.2f MB total data)\n", vector_size, size_mb);
    printf("Iterations:  %d\n\n", iterations);

    double i_total_time = 0;
    struct timespec start_real, end_real;
    // Start timing
    timespec_get(&start_real, TIME_UTC);
    clock_t start_time = clock();

    // Run AXPY many times, to make the time measurable
    // Will compare how that changes if doing per-loop checks
    for (int iter = 0; iter < iterations; iter++) {
        clock_t i_start_time = clock();
        one_axpy(vector_size, Z, X, Y, alpha);
        clock_t i_end_time = clock();
        i_total_time += (double)(i_end_time - i_start_time) / CLOCKS_PER_SEC;
        // make a change to the input parameters, to avoid compiler over-optimization
	
        // TODO: Some of the data may not be in my local buffer slice
	//       If needed, send to the right process, or fetch from the right process
        X[iter%vector_size] = Z[0];
        Y[0] = Z[iter%vector_size];
    }

    // End timing
    clock_t end_time = clock();
    timespec_get(&end_real, TIME_UTC);

    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    double real_time = (end_real.tv_sec - start_real.tv_sec) +
                       (end_real.tv_nsec - start_real.tv_nsec) / 1e9;

    // Print results
    // TODO: Only one process should print out
    printf("Total time:  %0.4f seconds (used %0.4f CPU seconds, partials sum: %0.4f)\n", real_time, total_time, i_total_time);

    // Prevent compiler from optimizing away the whole loop by using the result

    // TODO: I don't have all the data, collect and aggregate partial sums from the other processes
    float sum = 0;
    for (int i=0;i<vector_size; i++) sum += Z[i];

    // TODO: Only one process should print out
    printf("Verification sum check: %f\n", sum);

    // Clean up
    free(Z);
    free(Y);
    free(X);

    // TODO: Finalize MPI

    return 0;
}

