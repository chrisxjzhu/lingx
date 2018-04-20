#include <lingx/core/event.h>
#include <lingx/core/connection.h>
#include <lingx/core/log.h>
#include <sys/socket.h>

namespace lnx {

ssize_t Unix_send(Connection* c, const void* buf, size_t size) noexcept
{
    Event* wev = c->write;

    for ( ;; ) {
        ssize_t n = ::send(c->fd, buf, size, 0);

        if (n > 0) {
            if (n < (ssize_t) size)
                wev->ready = 0;

            c->sent += n;

            return n;
        }

        if (n == 0) {
            Log_error(c->log, Log::ALERT, errno, "send() returned zero");
            wev->ready = 0;
            return n;
        }

        if (errno == EAGAIN || errno == EINTR) {
            wev->ready = 0;

            if (errno == EAGAIN)
                return AGAIN;

        } else {
            wev->error = 1;
            Connection_log_error(c, errno, "send() failed");
            return ERROR;
        }
    }
}

}
