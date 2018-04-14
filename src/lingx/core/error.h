#ifndef _LINGX_CORE_ERROR_H
#define _LINGX_CORE_ERROR_H

#include <cstddef>  // size_t
#include <exception>

namespace lnx {

const size_t MAX_ERROR_STR = 1024;

char* Strerror(int err, char* estr, size_t size) noexcept;

char* Strerrno(char* buf, const char* last, int err) noexcept;

class Error : public std::exception {
public:
    Error(int err, const char* fmt, ...) noexcept;

    const char* what() const noexcept override
    { return errstr_; }

private:
    char errstr_[MAX_ERROR_STR];
};

#define Throw_error(err, ...)  throw lnx::Error(err, __VA_ARGS__)

}

#endif
