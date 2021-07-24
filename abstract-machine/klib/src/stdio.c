#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int num_to_str(char* out, char* ctrl, int x)
{
    char temp[32];
    int len = 0, ans = 0;
    do {
        temp[len++] = x % 10 + '0';
        x /= 10;
    } while (x);
    if (ctrl[0] == '0' && ctrl[1] != '\0') {
        while (len < ctrl[1] - '0')
            temp[len++] = '0';
        //assert(len == ctrl[1] - '0');
    }
    ans = len;
    memset(ctrl, '\0', sizeof(ctrl));
    while (len > 0)
        *out++ = temp[--len];
    return ans;
}

int _Printf(char* out, const char* fmt, va_list args)
{

    int temp = 0, ctrlcnt = 0;
    char* initout = out;
    char* strtemp = '\0';
    char ctrl[8];
    while (*fmt) {
        if (*fmt != '%') {
            *out++ = *fmt++;
            continue;
        }
        fmt++;
        ctrlcnt = 0;
        while (*fmt >= '0' && *fmt <= '9')
            ctrl[ctrlcnt++] = *fmt++;
        switch (*fmt++) {
        case 'd':
            temp = num_to_str(out, ctrl, va_arg(args, int));
            out += temp;
            break;
        case 'c':
            *out++ = va_arg(args, int);
            break;
        case 's':
            strtemp = va_arg(args, char*);
            while (*strtemp)
                *out++ = *strtemp++;
            break;
        }
    }
    return out - initout;
}

int printf(const char* fmt, ...)
{
    va_list ap;
    char buf[256];
    va_start(ap, fmt);
    int ans = _Printf(buf, fmt, ap);
    buf[ans] = '\0';
    va_end(ap);
    size_t cnt = 0;
    while (buf[cnt]) {
        putch(buf[cnt]);
        buf[cnt] = '\0';
        cnt++;
    }
    return ans;
}

int vsprintf(char* out, const char* fmt, va_list ap)
{
    panic("Not implemented");
}

int sprintf(char* out, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ans = _Printf(out, fmt, ap);
    out[ans] = '\0';
    va_end(ap);
    return ans;
}

int snprintf(char* out, size_t n, const char* fmt, ...)
{
    panic("Not implemented");
}

int vsnprintf(char* out, size_t n, const char* fmt, va_list ap)
{
    panic("Not implemented");
}

#endif
