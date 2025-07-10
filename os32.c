#include <stdio.h>
#include <omp.h>
#include <time.h>

#define N 10000000
#define BLOCK_SIZE 3316130
#define NUM_THREADS 16

int main() {
    omp_set_num_threads(NUM_THREADS);
    
    double pi = 0.0;
    double sum = 0.0;
    const double delta = 1.0 / N;
    
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for reduction(+:sum) schedule(dynamic, BLOCK_SIZE)
    for (int i = 0; i < N; i++) {
        double x = (i + 0.5) * delta;
        sum += 4.0 / (1.0 + x * x);
    }
    
    pi = sum * delta;
    
    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;
    
    printf("Calculated pi = %.15f\n", pi);
    printf("Threads used: %d\n", NUM_THREADS);
    printf("Execution time: %.5f seconds\n", elapsed_time);
    return 0;
}