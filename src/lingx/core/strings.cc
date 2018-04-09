#include <lingx/core/strings.h>
#include <cstdarg>

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

}
