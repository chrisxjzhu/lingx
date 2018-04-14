#include <lingx/core/strings.h>
#include <cstdarg>
#include <climits>  // INT_MAX

namespace lnx {

char* Strncpy(char* dst, const char* src, size_t n) noexcept
{
    if (n == 0)
        return dst;

    for (; --n; ++dst, ++src) {
        *dst = *src;

        if (*dst == '\0')
            return dst;
    }

    *dst = '\0';

    return dst;
}

char* Slprintf(char* buf, const char* last, const char* fmt, ...) noexcept
{
    va_list args;

    va_start(args, fmt);
    buf = Vslprintf(buf, last, fmt, args);
    va_end(args);

    return buf;
}

char* Vslprintf(char* buf, const char* last, const char* fmt, va_list args) noexcept
{
    int len = 0;
    int size = last - buf;

    if (size > 0) {
        len = std::vsnprintf(buf, size, fmt, args);
        if (len >= size)
            len = size - 1;
    }

    /* discard error */
    return len > 0 ? buf + len : buf;
}

int Atoi(const char* str, size_t n) noexcept
{
    if (n == 0)
        return -1;

    int val = 0;
    int cutoff = INT_MAX / 10;
    int cutlim = INT_MAX % 10;

    for (size_t i = 0; i < n; ++i) {
        if (str[i] < '0' || str[i] > '9')
            return -1;

        if (val >= cutoff && (val > cutoff || str[i] - '0' > cutlim))
            return -1;

        val = val * 10 + (str[i] - '0');
    }

    return val;
}

}
