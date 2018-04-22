#include <lingx/core/connection.h>
#include <lingx/core/log.h>

namespace lnx {

Os_io_t Io;

int Connection_log_error(const Connection* c, int err, const char* text) noexcept
{
    if (err == ECONNRESET && c->log_error == ERROR_IGNORE_ECONNRESET)
        return 0;

    Log::Level level;

    if (err == 0
        || err == ECONNRESET
        || err == EPIPE
        || err == ENOTCONN
        || err == ETIMEDOUT
        || err == ECONNREFUSED
        || err == ENETDOWN
        || err == ENETUNREACH
        || err == EHOSTDOWN
        || err == EHOSTUNREACH)
    {
        switch (c->log_error) {
        case ERROR_IGNORE_EINVAL:
        case ERROR_IGNORE_ECONNRESET:
        case ERROR_INFO:
            level = Log::INFO;
            break;
        default:
            level = Log::ERROR;
        }
    } else {
        level = Log::ALERT;
    }

    Log_error(c->log, level, err, text);

    return ERROR;
}

}
