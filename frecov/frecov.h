#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define _Log(format, ...)                            \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
           __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define _panic(cond, format, ...)                                                                      \
    if (!cond)                                                                                         \
    {                                                                                                  \
        printf("\33[1;31m[%s,%d,%s] " format "\33[0m\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        exit(0);                                                                                       \
    }

#ifdef DEBUG
#define Log(...) _Log(__VA_ARGS__)
#define panic(...) _panic(__VA_ARGS__)
#else
#define Log(...)
#define panic(...)
#endif