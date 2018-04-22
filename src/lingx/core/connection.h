#ifndef _LINGX_CORE_CONNECTION
#define _LINGX_CORE_CONNECTION

#include <lingx/core/common.h>
#include <lingx/core/os_io.h>

namespace lnx {

enum LogError {
    ERROR_ALERT = 0,
    ERROR_ERR,
    ERROR_INFO,
    ERROR_IGNORE_ECONNRESET,
    ERROR_IGNORE_EINVAL
};

struct Connection {
    void* data;

    Event* read;
    Event* write;

    int fd;

    off_t sent;

    Log* log;

    unsigned log_error:3;
};

int Connection_log_error(const Connection* c, int err, const char* text) noexcept;

}

#endif
