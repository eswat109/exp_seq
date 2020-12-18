#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif
#include "config.h"
#pragma once

EXTERN_C double omp_accumulate_false_sharing(const double* V, size_t count);
EXTERN_C double omp_accumulate_aligned(const double* V, size_t count);
EXTERN_C double omp_accumulate_atomic(const double* V, size_t count);
EXTERN_C double omp_accumulate_critical(const double* V, size_t count);
EXTERN_C double omp_accumulate_reduction_static(const double* V, size_t count);
EXTERN_C double omp_accumulate_reduction_dynamic(const double* V, size_t count);

double cpp_accumulate_false_sharing(const double *V, size_t count);
double cpp_accumulate_aligned(const double *V, size_t count);
double cpp_accumulate_atomic(const double *V, size_t count);
double cpp_accumulate_mutex(const double *V, size_t count);
double cpp_accumulate_reduction_static(const double *V, size_t count);
