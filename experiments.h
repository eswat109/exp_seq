#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif
#pragma once

typedef struct experiment_result_ {
    double result;
    double time_ms;
    double speedup;
} experiment_result;

experiment_result *run_experiments(const double *V, size_t count, double (*accumulate)(const double *, size_t));
void print_experiment_results(const experiment_result *results);
void run_experiments_for(const double *V, size_t count, double (*accumulate)(const double *, size_t));
