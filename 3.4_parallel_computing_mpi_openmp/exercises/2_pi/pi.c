/*
 * Simple PI benchmark test.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Default values if no command line arguments are provided
#define DEFAULT_PI_ITERATIONS 1000000
#define DEFAULT_BENCH_ITERATIONS 500
#define DEFAULT_NOISE 1.e-8

// Use Leibniz formula
double one_pi(double start_noise,int pi_iterations) {
    double pi = start_noise; // to avoid compiler over-optimization

    // Main compute loop
    //
    // Student TODO: Add OpenMP parallelization here
    for (int i = 0; i < (pi_iterations-1); i+=2) {
        pi += (4.0 / (2.0*i+1.0));
        pi -= (4.0 / (2.0*(i+1)+1.0));
    }
    if ((pi_iterations%2)!=0) { // deal with the odd case
        pi += (4.0 / (2.0*(pi_iterations-1)+1.0));
    }
    return pi;
}

int main(int argc, char *argv[]) {
    int pi_iterations = DEFAULT_PI_ITERATIONS; // the higher the number, the more precise PI
    int bench_iterations = DEFAULT_BENCH_ITERATIONS; // to make the times measurables
    double noise = DEFAULT_NOISE; // to avoid compiler over-optimization

    // Parse command line arguments if provided
    if (argc >= 2) {
        pi_iterations = atoi(argv[1]);
        if (pi_iterations <= 0) pi_iterations = DEFAULT_PI_ITERATIONS;
    }
    if (argc >= 3) {
        bench_iterations = atoi(argv[2]);
        if (bench_iterations <= 0) bench_iterations = DEFAULT_BENCH_ITERATIONS;
    }
    if (argc >= 4) {
        noise = atof(argv[3]);
    }

    double *pi_arr = (double *)malloc(bench_iterations * sizeof(double));
    // ignore error checking, let it crash if malloc fails
    

    // Initialize vector with dummy data
    for (int i = 0; i < bench_iterations; i++) {
        pi_arr[i] = sin((float)i) * noise;
    }
    printf("Running PI benchmark...\n");
    printf("Precision:   %d\n", pi_iterations);
    printf("Iterations:  %d\n", bench_iterations);

    struct timespec start_real, end_real;
    // Start timing
    timespec_get(&start_real, TIME_UTC);
    clock_t start_time = clock();


    // Run PI many times, to make the time measurable
    //
    // Note: This is the alternative place to parallelize using OpenMP
    for (int iter = 0; iter < bench_iterations; iter++) {
        pi_arr[iter] = one_pi(pi_arr[iter], pi_iterations);
    }

    // End timing
    clock_t end_time = clock();
    timespec_get(&end_real, TIME_UTC);

    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    double real_time = (end_real.tv_sec - start_real.tv_sec) +
                       (end_real.tv_nsec - start_real.tv_nsec) / 1e9;

    // Print results
    printf("Total time:  %0.4f seconds (used %0.4f CPU seconds)\n", real_time, total_time);


    // Prevent compiler from optimizing away the whole loop by using the result
    double sum = 0;
    for (int i=0;i<bench_iterations; i++) sum += pi_arr[i];
    printf("Mean estimated value of Pi after %d iterations: %0.10f\n", pi_iterations, sum/bench_iterations);
    free(pi_arr);
    return 0;
}
