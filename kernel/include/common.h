#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define KB *(1L << 10)
#define MB *(1L << 20)
#define GB *(1L << 30)

#define Log(format, ...)                           \
    _Log("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
         __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DEBUG
#define Log(...) Log(__VA_ARGS__)
#else
#define Log(...)
#endif
