#include <pmm.h>

inline uint8_t log(size_t x)
{
    uint8_t ret = 0;
    while (x > 1)
    {
        x >>= 1;
        ret++;
    }
    return ret;
}

inline uint8_t calorder(size_t size)
{
    return ((log(size - 1) + 1));
}