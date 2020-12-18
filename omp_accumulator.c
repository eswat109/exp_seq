#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"

EXTERN_C double omp_accumulate_false_sharing(const double* V, size_t count)
{
	double full_sum = 0, *partial_sums;
	unsigned T;
	#pragma omp parallel shared(T, full_sum)
	{
		size_t t = (size_t) omp_get_thread_num();
		#pragma omp single
		{
			T = omp_get_num_threads();
			partial_sums = (double*) calloc(T, sizeof(double));
			verify(partial_sums);
		}
		for (size_t i = t; i < count; i += T)
			partial_sums[t] += V[i];
	}
	for (int i = 0; i < T; ++i)
		full_sum += partial_sums[i];
	free(partial_sums);
	return full_sum;
}

#ifndef _MSC_VER
#include <stdalign.h>
#define _aligned_free(x) free((x))
#else //_MSC_VER
#define alignas(val) __declspec(align(val))
#define aligned_alloc(alignment, size) _aligned_malloc((size), (alignment))
#endif //_MSC_VER

#define CACHE_LINE 64
struct aligned_double
{
	alignas(CACHE_LINE) double value;
};

EXTERN_C double omp_accumulate_aligned(const double* V, size_t count)
{
	double full_sum = 0;
	struct aligned_double* partial_sums;
	unsigned T;
	#pragma omp parallel shared(T)
	{
		size_t t = (size_t) omp_get_thread_num();
		#pragma omp single
		{
			T = omp_get_num_threads();
			partial_sums = (struct aligned_double*) aligned_alloc(CACHE_LINE, T * sizeof(struct aligned_double));
			verify(partial_sums);
		}
		partial_sums[t].value = 0;
		for (size_t i = t; i < count; i += T)
			partial_sums[t].value += V[i];
	}
	for (int i = 0; i < T; ++i)
		full_sum += partial_sums[i].value;
	_aligned_free(partial_sums);
	return full_sum;
}

double omp_accumulate_atomic(const double* V, size_t count)
{
	double sum = 0;
	int i;
	#pragma omp for
	for (i = 0; i < count; ++i)
		#pragma omp atomic
			sum += V[i];
	return sum;
}

double omp_accumulate_critical(const double* V, size_t count)
{
	double sum = 0;
	#pragma omp parallel
	{
		double local_sum = 0;
		for (size_t i = (size_t) omp_get_thread_num(); i < count; i += (size_t) omp_get_num_threads())
			local_sum += V[i];
		#pragma omp critical
		{
			sum += local_sum;
		}
	}
	return sum;
}

double omp_accumulate_reduction_static(const double* V, size_t count)
{
	double S = 0;
	int i = 0;
	#pragma omp parallel for reduction(+:S) schedule(static)
	for (i = 0; i < count; ++i)
		S += V[i];
	return S;
}

double omp_accumulate_reduction_dynamic(const double* V, size_t count)
{
	double S = 0;
	int i = 0;
	#pragma omp parallel for reduction(+:S) schedule(dynamic)
	for (i = 0; i < count; ++i)
		S += V[i];
	return S;
}
