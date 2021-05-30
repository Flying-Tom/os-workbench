#include <pmm.h>

inline size_t log(size_t x)
{
    size_t ret = 0;
    while (x > 1)
    {
        x >>= 1;
        ret++;
    }
    return ret;
}

size_t binalign(size_t size)
{
    bool flag = true;
    size_t ret = 1;
    assert(size > 0);
    while (size != 1)
    {
        if (size & 1)
            flag = false;
        ret <<= 1;
        size >>= 1;
    }
    ret = flag == true ? ret : ret << 1;
    return ret;
}