#include <iostream>
#include "num_threads.h"
#include "interface.h"
#include "experiments.h"
#include <math.h>

double accumuate_sequential(const double* V, size_t count)
{
	unsigned i;
	double S = 0;
	for (i = 0; i < count; ++i)
		S += V[i];
	return S;
}

double *generate_vector(size_t count) {
    double *V = (double *) malloc(count * sizeof(double));
    verify(V);
    for (size_t i = 0; i < count; ++i)
        V[i] = pow(-1,i)/(i+1);
    return V;
}

int main(int argc, char **argv) {
    size_t element_count = 10000000u;
    double *V = generate_vector(element_count);
    printf("Control: %f\n", accumuate_sequential(V, element_count));
    printf("------------С++-------------\n");
    printf("==False sharing==\n");
    run_experiments_for(V, element_count, cpp_accumulate_false_sharing);
    printf("==Alignment==\n");
    run_experiments_for(V, element_count, cpp_accumulate_aligned);
    printf("==Atomic==\n");
    run_experiments_for(V, element_count, cpp_accumulate_atomic);
    printf("==Critical==\n");
    run_experiments_for(V, element_count, cpp_accumulate_mutex);
    printf("==Reduction (static)==\n");
    run_experiments_for(V, element_count, cpp_accumulate_reduction_static);
    printf("------------OpenMP-------------\n");
    printf("==False sharing==\n");
	run_experiments_for(V, element_count, omp_accumulate_false_sharing);
	printf("==Alignment==\n");
	run_experiments_for(V, element_count, omp_accumulate_aligned);
	printf("==Atomic==\n");
	run_experiments_for(V, element_count, omp_accumulate_atomic);
	printf("==Critical==\n");
	run_experiments_for(V, element_count, omp_accumulate_critical);
	printf("==Reduction (static)==\n");
	run_experiments_for(V, element_count, omp_accumulate_reduction_static);
	printf("==Reduction (dynamic)==\n");
	run_experiments_for(V, element_count, omp_accumulate_reduction_dynamic);
    free(V);
    return 0;
}
