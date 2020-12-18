#pragma once

#ifdef __cplusplus
#define EXTERN_C extern "C"
#include <cstdio>
using std::fprintf;
#else
#define EXTERN_C
#include <stdio.h>
#endif

#define verify(bool_expr) if (!(bool_expr)) {fprintf(stderr, "Error in %s (%d)\n", __FILE__, __LINE__); abort();}
