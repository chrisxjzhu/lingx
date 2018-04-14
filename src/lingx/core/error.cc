#include <lingx/core/error.h>
#include <lingx/core/strings.h>
#include <system_error>
#include <cstdarg>

namespace lnx {

const std::error_category& Error_category_ = std::system_category();

char* Strerror(int err, char* estr, size_t size) noexcept
{
    const std::string& msg = Error_category_.default_error_condition(err)
                                            .message();

    return Strncpy(estr, msg.c_str(), size);
}

char* Strerrno(char* buf, const char* last, int err) noexcept
{
    if (buf + 50 > last) {
        /* leave space for errno message
         *
         * WARN: the caller must ensure the total buffer size is larger than
         *       50 chars. So we should always use a char[MAX_ERROR_STR]
         *       with this function.
         */
        buf = const_cast<char*>(last) - 50;

        for (int i = 0; i < 3; ++i)
            *buf++ = '.';
    }

    buf = Slprintf(buf, last, " (%d: ", err);

    buf = Strerror(err, buf, last - buf);

    if (buf < last)
        *buf++ = ')';

    return buf;
}

Error::Error(int err, const char* fmt, ...) noexcept
{
    const char* const last = errstr_ + sizeof(errstr_);
    char* p = errstr_;

    va_list args;
    va_start(args, fmt);
    p = Vslprintf(p, last, fmt, args);
    va_end(args);

    if (err)
        p = Strerrno(p, last, err);
}

}
