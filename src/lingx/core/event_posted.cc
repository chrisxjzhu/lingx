#include <lingx/core/event_posted.h>
#include <lingx/core/event.h>
#include <lingx/core/queue.h>

namespace lnx {

Queue  Posted_accept_events;
Queue  Posted_events;

void Post_event(Event* ev, Queue* q) noexcept
{
    if (!ev->posted) {
        ev->posted = 1;
        Queue_insert_tail(q, &ev->queue);
    }
}

}
