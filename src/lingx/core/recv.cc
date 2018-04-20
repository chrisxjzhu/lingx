#include <lingx/core/event.h>
#include <lingx/core/connection.h>
#include <sys/socket.h>

namespace lnx {

ssize_t Unix_recv(Connection* c, void* buf, size_t size) noexcept
{
    Event* rev = c->read;

    if (Event_flags & USE_EPOLL_EVENT) {
        if (!rev->available && !rev->pending_eof) {
            rev->ready = 0;
            return AGAIN;
        }
    }

    ssize_t n;

    do {
        n = ::recv(c->fd, buf, size, 0);

        if (n == 0) {
            rev->ready = 0;
            rev->eof = 1;
            return 0;
        }

        if (n > 0) {
            if ((Event_flags & USE_EPOLL_EVENT) && Use_epoll_rdhup) {
                if ((size_t) n < size) {
                    if (!rev->pending_eof)
                        rev->ready = 0;

                    rev->available = 0;
                }

                return n;
            }

            if ((size_t) n < size && !(Event_flags & USE_GREEDY_EVENT))
                rev->ready = 0;

            return n;
        }

        if (errno == EAGAIN || errno == EINTR) {
            n = AGAIN;
        } else {
            n = Connection_log_error(c, errno, "recv() failed");
            break;
        }
    } while (errno == EINTR);

    rev->ready = 0;

    if (n == ERROR)
        rev->error = 1;

    return n;
}

}
